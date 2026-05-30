# Gobang v2.0 — Rust 重写设计规格

## 概述

将 Gobang（五子棋）从 C + IUP + CMake 全面迁移到 Rust + Tauri + React + TypeScript，
参考 PathEditor 项目的 workspace 架构、工具链约定和开源规范。

## 技术栈

| 层 | 技术 |
|----|------|
| 核心引擎 | Rust (edition 2021, `stable-x86_64-pc-windows-gnu`) |
| 桌面框架 | Tauri 2.x |
| 前端 | React 19 + TypeScript strict + Vite |
| 网络 | renet（Rust 原生 ENet 协议实现） |
| 状态管理 | Zustand |
| 国际化 | i18next (zh-CN / en) |
| 包管理 | npm（前端）+ Cargo（Rust） |

## Crate 结构

```
Gobang/
├── core/                    # Rust 库 crate（纯逻辑，零 Tauri 依赖）
│   └── src/
│       ├── board.rs         # 棋盘状态、落子验证、胜负判定
│       ├── rules.rs         # 禁手规则
│       ├── ai/              # 传统算法 AI
│       │   ├── mod.rs
│       │   ├── evaluate.rs  # 棋形评分
│       │   └── search.rs    # Alpha-Beta 搜索
│       ├── network.rs       # renet 网络对战
│       ├── record.rs        # 棋谱记录与回放 (JSON)
│       ├── llm.rs           # 大模型 API 客户端
│       └── lib.rs
├── gui/                     # Tauri 桌面应用（薄层）
│   └── src/
│       ├── commands.rs      # #[tauri::command] → core API
│       ├── lib.rs
│       └── main.rs
├── src/                     # React 前端 (TypeScript strict)
│   ├── core/                # 前端纯逻辑（坐标转换、棋型常量）
│   ├── components/
│   │   ├── board/           # Canvas 棋盘渲染
│   │   ├── menu/            # 主菜单、设置
│   │   ├── game/            # 对局 UI
│   │   └── replay/          # 棋谱回放
│   ├── hooks/               # useGame, useTimer, useAI
│   ├── store/               # Zustand 状态管理
│   └── i18n/                # 中/英
├── Cargo.toml               # workspace 根
├── rust-toolchain.toml      # stable-x86_64-pc-windows-gnu
├── package.json
├── LICENSE                  # MIT
├── CHANGELOG.md
├── CODE_OF_CONDUCT.md
├── CONTRIBUTING.md
├── SECURITY.md
└── README.md
```

## 功能清单

v1.0 全部功能保留，Rust 重写：

| 模块 | 功能 | 实现位置 |
|------|------|----------|
| 本地双人 | 同机两人轮流落子 | core/board.rs |
| 传统 AI | 5 级难度、禁手规则、Alpha-Beta 搜索 | core/ai/ |
| LLM AI | 大模型 API 接入对战 | core/llm.rs |
| 网络对战 | renet P2P 联机 | core/network.rs |
| 棋谱回放 | 对局记录 (JSON) 与复盘 | core/record.rs |
| 计时器 | 每回合限时（可选） | gui + frontend |
| 悔棋 | 撤销指定步数 | core/board.rs |

## core crate 设计

### board.rs — 棋盘引擎

不可变风格核心抽象。每次落子返回新棋盘，通过 `Cow` 优化内存：

```rust
pub struct Board {
    size: usize,
    cells: [[CellState; MAX_SIZE]; MAX_SIZE],
}

pub enum CellState { Empty, Black, White }

impl Board {
    pub fn place(&self, pos: Position, color: Color) -> Result<Board, MoveError>;
    pub fn check_win(&self, pos: Position) -> bool;
    pub fn is_forbidden(&self, pos: Position) -> bool;
    pub fn get_candidate_moves(&self) -> Vec<Position>;
}
```

- `MoveError` 枚举：`OutOfBounds`, `Occupied`, `ForbiddenMove`, `GameOver`
- `place()` 不修改 self，返回新 Board 或错误
- 胜负判断：四方向连续扫描（复用 v1 的 DirInfo 思路，改为 Rust 迭代器）
- 禁手检测：长连、双三、双四检查

### ai/ 模块 — 传统 AI

Trait 抽象，支持 AI 引擎切换：

```rust
pub trait AiEngine: Send + Sync {
    fn best_move(&self, board: &Board, color: Color) -> Option<Position>;
}

pub struct AlphaBetaAi {
    depth: usize,
    defense_coefficient: f64,
}
```

- `evaluate.rs`：棋形打分（连五、活四、冲四、活三、眠三、活二）
- `search.rs`：Alpha-Beta 剪枝 + 迭代加深 + 启发式落子排序
- 难度 1-5 通过 depth 参数控制

### network.rs — renet 网络对战

```rust
pub enum GameMessage {
    Move { pos: Position, turn: u32 },
    Undo { steps: u32 },
    Resign,
    Chat(String),
}

pub struct NetworkSession {
    // renet client/server handle
}
```

- 服务端创建房间，客户端加入
- 可靠 UDP 传输，和原 ENet 行为一致
- 掉线检测 + 重连

### record.rs — 棋谱 (JSON)

```json
{
  "version": "2.0",
  "date": "2026-05-30",
  "board_size": 15,
  "players": { "black": "Human", "white": "AI-Lv3" },
  "moves": [{"x": 7, "y": 7, "color": "Black"}, ...],
  "result": "Black Win"
}
```

- 序列化/反序列化用 `serde` + `serde_json`
- 回放：按步加载 move 历史，前端逐帧展示

### llm.rs — 大模型 AI

- HTTP 客户端（`reqwest`）调用 OpenAI 兼容 API
- 将棋盘状态序列化为 prompt，解析模型回复的坐标
- 实现 `AiEngine` trait，可替换 AlphaBetaAi

## gui crate — Tauri 命令层

薄层，每个 `#[tauri::command]` 一行调用 core：

```rust
#[tauri::command]
fn place_piece(state: State<AppState>, x: usize, y: usize) -> Result<MoveResult, String> {
    let mut game = state.game.lock().map_err(|e| e.to_string())?;
    game.place(x, y).map_err(|e| e.to_string())
}
```

全局状态管理：

```rust
struct AppState {
    game: Mutex<Game>,
}

enum GameMode {
    Local,
    VsAi(Box<dyn AiEngine>),
    Online(NetworkSession),
    Replay(ReplayState),
}
```

IPC 命令清单：

| Command | 参数 | 返回值 | 功能 |
|---------|------|--------|------|
| `new_game` | `mode, board_size, config` | `Result<(), String>` | 开始新局 |
| `place_piece` | `x, y` | `Result<MoveResult, String>` | 落子 |
| `undo` | `steps` | `Result<(), String>` | 悔棋 |
| `get_board` | — | `Board` | 获取棋盘状态 |
| `ai_move` | — | `Result<Position, String>` | 请求 AI 走棋 |
| `connect_game` | `ip, port` | `Result<(), String>` | 网络连接 |
| `send_message` | `msg` | `Result<(), String>` | 网络消息 |
| `save_record` | `path` | `Result<(), String>` | 保存棋谱 |
| `load_record` | `path` | `Result<RecordData, String>` | 加载棋谱 |
| `get_game_config` | — | `GameConfig` | 读取设置 |

## 前端设计

### 组件树

```
App
├── MainMenu
│   ├── LocalGameSetup     # 本地双人
│   ├── AiGameSetup        # 难度、先手、禁手开关
│   ├── OnlineSetup        # IP:端口 / 创建房间
│   └── LoadReplay         # 加载棋谱文件
├── GameView
│   ├── BoardCanvas        # Canvas 棋盘 + 点击落子
│   ├── GameInfo           # 当前玩家、AI 思考中、胜负提示
│   ├── TimerDisplay       # 倒计时（可选）
│   └── GameControls       # 悔棋、认输、保存
└── ReplayView
    ├── BoardCanvas        # 只读棋盘
    ├── StepSlider         # 步数滑条
    └── ReplayControls     # 播放/暂停/快进
```

### 数据流

```
用户点击棋盘 → BoardCanvas.onClick
  → store.placePiece(x, y)
    → invoke('place_piece', {x, y})         # Tauri IPC
      → gui/commands.rs → core::Board::place()
    → store 更新 board / currentColor
    → React 重渲染 BoardCanvas
```

### Canvas 渲染要点

- `useRef<HTMLCanvasElement>` + `requestAnimationFrame`
- 棋盘线、棋子（渐变填充仿木纹风格）、最后一手高亮标记
- 坐标纯函数：`boardToCanvas(pos, cellSize, padding) → {x, y}`
- 支持窗口 resize 自适应棋盘大小

### 状态管理 (Zustand)

```typescript
interface GameStore {
  mode: GameMode;
  board: CellState[][];
  currentColor: Color;
  moves: Move[];
  gameStatus: 'waiting' | 'playing' | 'ai_thinking' | 'game_over';
  winner: Color | null;
  // actions
  startGame: (config: GameConfig) => Promise<void>;
  placePiece: (x: number, y: number) => Promise<void>;
  undo: () => Promise<void>;
  loadReplay: (path: string) => Promise<void>;
}
```

## 错误处理策略

| 层 | 策略 |
|----|------|
| **core** | `Result<T, MoveError>` / `Result<T, GameError>`，中文错误消息 |
| **gui** | `Result<T, String>`，`to_string()` 转换，Tauri 自动序列化 |
| **前端** | `.catch()` 显示错误 toast，`isSaving` 防并发点击 |

## 构建与测试

```bash
# 开发模式
npm install
npx tauri dev

# Rust 检查
cargo check
cargo clippy -- -D warnings
cargo fmt --check

# 测试
cargo test                    # Rust 单元测试
npm test                      # 前端单元测试 (Vitest)

# 生产构建
npx tauri build
```

### 测试覆盖率目标：80%+

## 与 PathEditor 一致的设计决策

- **工具链**：`stable-x86_64-pc-windows-gnu`
- **Workspace**：`resolver = "2"`, edition 2021
- **前端**：TypeScript strict, Vitest, i18next
- **开源文件**：LICENSE (MIT), CHANGELOG.md, CODE_OF_CONDUCT.md, CONTRIBUTING.md, SECURITY.md
- **提交格式**：约定式提交（`feat:`, `fix:`, `refactor:` 等）
- **版本管理**：`Cargo.toml` workspace.package.version + `package.json` version 集中管理
