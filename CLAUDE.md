# CLAUDE.md

## 项目概述

Gobang v2.0 — 五子棋桌面应用，Rust + Tauri 2.x + React 19 + TypeScript workspace 构建。

## 构建命令

```bash
# 安装前端依赖
npm install

# 开发模式（GUI 热更新）
npx tauri dev

# 仅前端（浏览器预览，无 Tauri 功能）
npm run dev

# 前端测试
npm test
npm run test:watch

# Rust workspace 全部检查
cargo check

# 仅核心库检查
cargo check -p gobang-core

# Rust 测试
cargo test

# Lint
cargo clippy -- -D warnings

# Rust 格式化
cargo fmt

# 前端类型检查
npx tsc -b

# 生产构建（生成 NSIS 安装包）
npx tauri build
```

## 架构

Cargo workspace 两 crate，前后端分离，通过 Tauri IPC 通信。

```
Gobang/
├── core/                       # Rust 库 crate（零 Tauri 依赖）
│   └── src/
│       ├── types.rs            # 基础类型 (Position, Color, CellState, GameConfig)
│       ├── board.rs            # 棋盘引擎 (落子/胜负/悔棋/候选位)
│       ├── rules.rs            # 禁手规则 (长连/双三/双四)
│       ├── ai/
│       │   ├── mod.rs          # AiEngine trait
│       │   ├── evaluate.rs     # 棋形评分
│       │   └── search.rs       # Alpha-Beta 搜索
│       ├── record.rs           # JSON 棋谱记录与复盘
│       ├── network.rs          # renet 网络对战协议
│       └── llm.rs              # LLM AI (OpenAI 兼容 API)
├── gui/                        # Tauri 桌面应用（依赖 core）
│   └── src/
│       ├── main.rs             # 入口
│       ├── lib.rs              # Tauri Builder + AppState 注册
│       └── commands.rs         # #[tauri::command] → 调用 core
├── src/                        # React 前端 (TypeScript strict 模式)
│   ├── core/                   # 纯逻辑 (types.ts, constants.ts)
│   ├── store/                  # Zustand 状态管理
│   ├── components/
│   │   ├── board/              # BoardCanvas + board-renderer
│   │   ├── menu/               # MainMenu + 各模式 setup
│   │   ├── game/               # GameView / GameInfo / TimerDisplay / GameControls
│   │   └── replay/             # ReplayView / StepSlider / ReplayControls
│   ├── hooks/                  # 通用 hooks
│   └── i18n/                   # zh-CN / en
├── rust-toolchain.toml         # 固定工具链版本
└── Cargo.toml                  # Workspace 根 + [workspace.package]
```

## IPC 接口（Rust → Frontend）

| Command | 参数 | 返回值 | 功能 |
|---------|------|--------|------|
| `new_game` | `mode: GameMode, config: GameConfig` | `Result<(), String>` | 开始新局 |
| `place_piece` | `x: usize, y: usize` | `Result<MoveResult, String>` | 落子 |
| `undo` | `steps: u32` | `Result<(), String>` | 悔棋 |
| `get_board` | — | `Result<Vec<Vec<i32>>, String>` | 获取棋盘 |
| `ai_move` | — | `Result<Option<(usize, usize)>, String>` | AI 走棋 |
| `get_game_state` | — | `Result<Value, String>` | 完整游戏状态 |

## Serde 序列化规则

- `GameConfig` 使用 `#[serde(rename_all = "camelCase")]` — 前端 camelCase ↔ Rust snake_case
- `GameMode` 无 rename — 保持 PascalCase (`Local`, `VsAi`, `Online`, `Replay`)
- `Color` 无 rename — 保持 PascalCase (`Black`, `White`)

## 错误处理

### 前端

| 场景 | 处理 |
|------|------|
| IPC 调用失败 | 返回 `Result<_, String>`，前端 `.catch()` 显示错误 |
| 落子并发双击 | `GameControls` 检查 `status === 'game_over'` 禁用按钮 |
| JSON 棋谱损坏 | `LoadReplay` try/catch，alert 提示 |
| 渲染异常 | Canvas `useRef` + 条件渲染兜底 |

### Rust

- Board 操作：`Result<Board, MoveError>`，中文 Display
- IPC 命令：`Result<T, String>`，`map_err(|e| e.to_string())`
- AI 禁手：`best_move()` 和 `negamax()` 内调用 `rules::is_forbidden()` 过滤
- LLM 阻塞：`best_move()` 使用 `reqwest::blocking`，调用方应通过 `spawn_blocking` 包装

## 关键约束

- **Rust 工具链**：`stable-x86_64-pc-windows-gnu`（`rust-toolchain.toml` 强制）
- **TypeScript**：`strict: true`，零编译错误
- **打包**：NSIS（`targets: "nsis"`），不使用 WiX/MSI
- **不可变风格**：`Board::place()` 和 `undo()` 返回新 Board，不修改 self
- **i18n 初始化**：`main.tsx` 必须 `import './i18n'` 引入，否则界面显示 key

## 版本号升级清单

| 文件 | 字段 | 说明 |
|------|------|------|
| `Cargo.toml` | `[workspace.package] version` | Rust 端版本 |
| `package.json` | `version` | 前端版本 |
| `gui/tauri.conf.json` | `version` | 打包版本号 |
| `gui/tauri.conf.json` | 窗口 `title` | 标题栏显示 |
