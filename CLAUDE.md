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
│       ├── types.rs            # 基础类型：Color, Position, CellState, GameConfig
│       │                       #   GameConfig = GameRulesConfig + AiConfig + NetworkConfig
│       │                       #   三个子结构体用 #[serde(flatten)] 展平为单一 JSON
│       ├── board.rs            # 棋盘引擎：不可变 place/undo，Zobrist 增量哈希
│       ├── rules.rs            # 禁手规则：长连(≥6)、严格活三(可延伸活四)、双四
│       ├── scan.rs             # 公共方向线段扫描 → 被 rules/evaluate/vcf 复用
│       ├── ai/
│       │   ├── mod.rs          # AiEngine trait (Send + Sync)
│       │   ├── search.rs       # 迭代加深 Alpha-Beta + Negamax，5 档难度(1-8s)
│       │   ├── evaluate.rs     # 棋形评分 + 组合棋形(双三/三四/双四) + 位置权重
│       │   ├── trans_table.rs  # 置换表 1M 条目，Zobrist 索引，depth 优先替换
│       │   ├── killer.rs       # Killer move 启发式，每层 2 槽位 + LRU 淘汰
│       │   ├── opening.rs      # 开局库 50 定式，Zobrist 索引 + 随机选取
│       │   └── vcf.rs          # VCF(连续冲四)/VCT(活三冲四混合) 杀棋搜索
│       ├── llm.rs              # LLM AI：SSE 流式调用 + 正则/逐字节坐标解析
│       ├── network.rs          # renet2 UDP 网络对战，断线重连(3次/15s超时)
│       └── record.rs           # JSON 棋谱记录与复盘 (chrono ISO 8601)
├── gui/                        # Tauri 桌面应用（依赖 core）
│   └── src/
│       ├── main.rs             # 入口，env_logger 初始化
│       ├── lib.rs              # Tauri Builder + AppState 注册 + 14 命令 handler
│       └── commands.rs         # #[tauri::command] → 调用 core
│           #   AppState = RwLock<GameSession> + ai_engine Mutex + ai_busy Mutex + network_tx Mutex
│           #   GameSession 合并 board/mode/config/color/game_over/winner
├── src/                        # React 前端 (TypeScript strict 模式)
│   ├── core/                   # 纯逻辑：types.ts (GameConfig extends 3 子接口), constants.ts
│   ├── store/                  # Zustand 状态管理 (13 字段 + 10 actions)
│   ├── components/
│   │   ├── board/              # BoardCanvas + board-renderer (Canvas 渲染)
│   │   ├── menu/               # MainMenu + LocalGame/AiGame/Online/LoadReplay setup
│   │   ├── game/               # GameView / GameInfo / TimerDisplay / GameControls
│   │   ├── replay/             # ReplayView / StepSlider / ReplayControls
│   │   └── common/             # ErrorBoundary
│   ├── i18n/                   # zh-CN / en (i18next)
│   ├── App.tsx                 # 根组件 (menu/game/replay 三页面)
│   └── main.tsx                # 入口 (StrictMode + import './i18n')
├── .github/workflows/ci.yml    # CI: Rust check/test/clippy/fmt + 前端 tsc/test
├── rust-toolchain.toml         # 固定工具链 stable-x86_64-pc-windows-gnu
└── Cargo.toml                  # Workspace 根，[workspace.dependencies] 集中版本管理
```

## IPC 接口（Rust → Frontend）

| Command | 参数 | 返回值 | 功能 |
|---------|------|--------|------|
| `new_game` | `mode: GameMode, config: GameConfig` | `Result<(), String>` | 开始新局，初始化 AI |
| `place_piece` | `x: usize, y: usize` | `Result<MoveResult, String>` | 落子（禁手检查 + 胜负判定 + winner 设置） |
| `undo` | `steps: u32` | `Result<(), String>` | 悔棋（恢复 color/winner/game_over） |
| `ai_move` | — | `Result<Option<(usize, usize)>, String>` | Alpha-Beta AI 走棋（独立线程，30s 超时，ai_busy 去重） |
| `ai_move_llm` | — | `Result<Option<(usize, usize)>, String>` | LLM AI 走棋（SSE 流式，Tauri 事件 `llm-stream` 推送 token） |
| `get_game_state` | — | `Result<Value, String>` | 完整游戏状态（board/color/game_over/winner） |
| `resign` | — | `Result<(), String>` | 认输（设置 winner 为对手） |
| `save_record` | — | `Result<String, String>` | 导出 JSON 棋谱 |
| `host_game` | `port: u16` | `Result<u16, String>` | 创建网络房间（返回实际端口） |
| `join_game` | `address: String` | `Result<(), String>` | 加入网络房间 |
| `send_move` | `x: usize, y: usize, turn: u32` | `Result<(), String>` | 发送网络走棋 |
| `send_undo` | `steps: u32` | `Result<(), String>` | 发送网络悔棋 |
| `send_resign` | — | `Result<(), String>` | 发送网络认输 |

## Serde 序列化规则

- `GameConfig` 拆分为 `GameRulesConfig` + `AiConfig` + `NetworkConfig`，用 `#[serde(flatten)]` 保持 JSON 扁平结构
- 所有子结构体均使用 `#[serde(rename_all = "camelCase")]` — 前端 camelCase ↔ Rust snake_case
- `GameMode` 无 rename — 保持 PascalCase (`Local`, `VsAi`, `Online`, `Replay`)
- `Color` 无 rename — 保持 PascalCase (`Black`, `White`)
- 前端 `GameConfig extends GameRulesConfig, AiConfig, NetworkConfig` 与 Rust 对齐

## 关键数据结构

### GameSession（Rust gui 层）

```rust
struct GameSession {
    board: Option<Board>,       // 当前棋盘
    mode: GameMode,             // 游戏模式
    config: GameConfig,         // 游戏配置
    current_color: Color,       // 当前执子方
    game_over: bool,            // 是否已结束
    winner: Option<Color>,      // 胜者（None=未结束/平局）
}
```

### AppState（Rust gui 层）

```rust
pub struct AppState {
    pub session: RwLock<GameSession>,                           // 高频读写合并
    pub ai_engine: Mutex<Option<Arc<dyn AiEngine + Send + Sync>>>, // AI 实例
    pub ai_busy: Mutex<bool>,                                   // AI 去重
    pub network_tx: Mutex<Option<mpsc::Sender<NetworkCmd>>>,    // 网络命令通道
}
```

## 错误处理

### 前端

| 场景 | 处理 |
|------|------|
| IPC 调用失败 | 返回 `Result<_, String>`，前端 `.catch()` 显示错误 |
| 落子并发双击 | `GameControls` 检查 `status === 'game_over'` 禁用按钮 |
| JSON 棋谱损坏 | `LoadReplay` try/catch，alert 提示 |
| 渲染异常 | Canvas `useRef` + `ErrorBoundary` 兜底 |
| AI 重复调用 | Rust 端 `ai_busy` 去重，返回 "AI 正在计算中" |

### Rust

- Board 操作：`Result<Board, MoveError>`，中文 Display
- IPC 命令：`Result<T, String>`，`map_err(|e| e.to_string())`
- AI 禁手：`best_move()` 和 `negamax()` 内调用 `rules::is_forbidden()` 过滤
- LLM 同步调用：`Handle::try_current()` 失败时 `log::error!` 记录，返回 None
- LLM 异步调用（`ai_move_llm`）：通过 Tauri 事件流式推送，推荐前端优先使用
- 网络断线：client 端 15s 超时 + 3 次重试后发送 Error 事件退出

## 关键约束

- **Rust 工具链**：`stable-x86_64-pc-windows-gnu`（`rust-toolchain.toml` 强制）
- **TypeScript**：`strict: true`，零编译错误
- **打包**：NSIS（`targets: "nsis"`），不使用 WiX/MSI
- **不可变风格**：`Board::place()` 和 `undo()` 返回新 Board，不修改 self
- **i18n 初始化**：`main.tsx` 必须 `import './i18n'` 引入，否则界面显示 key
- **AI 引擎**：通过 `AiEngine` trait 抽象，当前实现 `AlphaBetaAi` 和 `LlmAi`
- **公共扫描**：`scan.rs` 是 `rules.rs`/`evaluate.rs`/`vcf.rs` 的唯一扫描入口，禁止各自实现
- **配置拆分**：`GameConfig` = `GameRulesConfig` + `AiConfig` + `NetworkConfig`，`#[serde(flatten)]` 展平

## 版本号升级清单

| 文件 | 字段 | 说明 |
|------|------|------|
| `Cargo.toml` | `[workspace.package] version` | Rust 端版本 |
| `package.json` | `version` | 前端版本 |
| `gui/tauri.conf.json` | `version` | 打包版本号 |
| `gui/tauri.conf.json` | 窗口 `title` | 标题栏显示 |
| `gui/tauri.conf.json` | `identifier` | 应用标识符 |
| `README.md` | shields.io badge | 版本号徽章 |
