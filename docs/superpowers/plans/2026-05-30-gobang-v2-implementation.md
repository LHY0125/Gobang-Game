# Gobang v2.0 Rust 重写 — 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 Gobang（五子棋）从 C + IUP + CMake 全面重写为 Rust + Tauri + React + TypeScript 架构。

**Architecture:** Cargo workspace 两 crate（core/gui）加 React 前端。core 是纯 Rust 库（零 GUI 依赖），gui 是 Tauri 薄命令层，src/ 是 React TypeScript strict 模式前端。数据流：React → Tauri IPC → gui/commands → core API。

**Tech Stack:** Rust edition 2021, Tauri 2.x, React 19, TypeScript strict, Vite, Zustand, i18next, renet

**源参考:** 读取 `D:\Code\doing_exercises\programs\PathEditor` 项目的对应文件获取完整代码模板。所有开源文件（LICENSE、CHANGELOG.md、CODE_OF_CONDUCT.md、CONTRIBUTING.md、SECURITY.md）均以 PathEditor 对应文件为基础，替换项目名称和上下文。

---

## 文件结构

```
Gobang/
├── core/
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs              # pub mod 声明, re-exports
│       ├── types.rs            # Position, Color, CellState, Move, GameResult
│       ├── board.rs            # Board 结构体, place, check_win, undo, get_candidate_moves
│       ├── rules.rs            # is_forbidden, 禁手模式检测
│       ├── ai/
│       │   ├── mod.rs          # AiEngine trait, AiFactory
│       │   ├── evaluate.rs     # 棋形评分函数 (活三/冲四/连五等)
│       │   └── search.rs       # AlphaBetaAi: Alpha-Beta 剪枝 + 迭代加深
│       ├── record.rs           # GameRecord serde, save/load JSON
│       ├── network.rs          # NetworkSession, GameMessage, renet 封装
│       └── llm.rs              # LlmAi: reqwest HTTP client, prompt 构建
├── gui/
│   ├── Cargo.toml
│   ├── tauri.conf.json
│   ├── build.rs
│   ├── icons/                  # 应用图标 (tauri icon 生成)
│   └── src/
│       ├── main.rs             # Tauri 入口
│       ├── lib.rs              # setup(), AppState, GameMode 枚举
│       └── commands.rs         # 所有 #[tauri::command] 函数
├── src/                        # React 前端
│   ├── core/
│   │   ├── types.ts            # 前端类型定义 (镜像 Rust 类型)
│   │   └── constants.ts        # 棋盘常量
│   ├── store/
│   │   └── gameStore.ts        # Zustand store
│   ├── hooks/
│   │   ├── useGame.ts          # 游戏逻辑 hook
│   │   └── useTimer.ts         # 计时器 hook
│   ├── components/
│   │   ├── board/
│   │   │   ├── BoardCanvas.tsx # Canvas 棋盘组件
│   │   │   └── board-renderer.ts # 纯函数: 绘制棋盘线/棋子/高亮
│   │   ├── menu/
│   │   │   ├── MainMenu.tsx    # 主菜单导航
│   │   │   ├── LocalGameSetup.tsx
│   │   │   ├── AiGameSetup.tsx
│   │   │   ├── OnlineSetup.tsx
│   │   │   └── LoadReplay.tsx
│   │   ├── game/
│   │   │   ├── GameView.tsx    # 对局主视图
│   │   │   ├── GameInfo.tsx    # 状态栏
│   │   │   ├── TimerDisplay.tsx
│   │   │   └── GameControls.tsx
│   │   └── replay/
│   │       ├── ReplayView.tsx
│   │       ├── StepSlider.tsx
│   │       └── ReplayControls.tsx
│   ├── i18n/
│   │   ├── index.ts
│   │   ├── zh-CN.json
│   │   └── en.json
│   ├── App.tsx
│   ├── App.css
│   ├── main.tsx
│   └── index.css
├── index.html                  # Vite 入口 HTML
├── package.json
├── tsconfig.json
├── tsconfig.node.json
├── vite.config.ts
├── Cargo.toml                  # workspace 根
├── rust-toolchain.toml
├── .gitignore
├── LICENSE                     # MIT (从 PathEditor 复制, 替换项目名)
├── CHANGELOG.md                # v2.0.0 条目
├── CODE_OF_CONDUCT.md          # 从 PathEditor 复制
├── CONTRIBUTING.md             # 从 PathEditor 复制, 改为 Gobang 上下文
├── SECURITY.md                 # 从 PathEditor 复制, 改为 Gobang 上下文
└── README.md                   # 重写为 Gobang v2.0 介绍
```

---

### Task 1: 项目脚手架 — Cargo workspace + Rust 基础

**Files:**
- Create: `Cargo.toml`
- Create: `rust-toolchain.toml`
- Create: `core/Cargo.toml`
- Create: `core/src/lib.rs`
- Create: `.gitignore`

- [ ] **Step 1: 创建 workspace 根 Cargo.toml**

```toml
[workspace]
resolver = "2"
members = [
    "core",
    "gui",
]

[workspace.package]
version = "2.0.0"
edition = "2021"
license = "MIT"
authors = ["刘航宇"]
repository = "https://github.com/LHY0125/Gobang"
```

文件路径: `Cargo.toml`

- [ ] **Step 2: 创建 rust-toolchain.toml**

```toml
[toolchain]
channel = "stable-x86_64-pc-windows-gnu"
```

文件路径: `rust-toolchain.toml`

- [ ] **Step 3: 创建 core/Cargo.toml**

```toml
[package]
name = "gobang-core"
version.workspace = true
edition.workspace = true
license.workspace = true
authors.workspace = true
repository.workspace = true

[dependencies]
serde = { version = "1", features = ["derive"] }
serde_json = "1"
renet = "0.6"
reqwest = { version = "0.12", features = ["json", "blocking"] }
rand = "0.8"
```

文件路径: `core/Cargo.toml`

- [ ] **Step 4: 创建 core/src/lib.rs (空壳)**

```rust
// Gobang core library — 纯游戏逻辑，零 GUI 依赖
```

文件路径: `core/src/lib.rs`

- [ ] **Step 5: 创建 .gitignore**

```
node_modules
dist
dist-ssr
*.local
target/
.vscode/*
!.vscode/extensions.json
.idea
.DS_Store
*.suo
*.sw?
.claude/
.codegraph/
CLAUDE.md
```

文件路径: `.gitignore`

- [ ] **Step 6: 验证 Rust 编译**

```bash
cargo check
```

Expected: `core` crate 编译成功，无错误。

- [ ] **Step 7: 提交**

```bash
git add Cargo.toml rust-toolchain.toml .gitignore core/Cargo.toml core/src/lib.rs
git commit -m "feat: 初始化 Cargo workspace + core crate 脚手架"
```

---

### Task 2: core/types.rs — 基础类型定义

**Files:**
- Create: `core/src/types.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 编写类型定义**

```rust
use serde::{Deserialize, Serialize};

/// 棋盘最大尺寸
pub const MAX_BOARD_SIZE: usize = 19;

/// 棋子颜色
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Color {
    Black,
    White,
}

impl Color {
    /// 切换颜色 (黑→白, 白→黑)
    pub fn opponent(self) -> Self {
        match self {
            Color::Black => Color::White,
            Color::White => Color::Black,
        }
    }
}

/// 棋盘位置 (0-based)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Position {
    pub x: usize,
    pub y: usize,
}

impl Position {
    pub fn new(x: usize, y: usize) -> Self {
        Self { x, y }
    }
}

use std::cmp::Ordering;

impl PartialOrd for Position {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Position {
    fn cmp(&self, other: &Self) -> Ordering {
        self.x.cmp(&other.x).then(self.y.cmp(&other.y))
    }
}

/// 棋盘格状态
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum CellState {
    Empty,
    Occupied(Color),
}

/// 一步棋
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Move {
    pub position: Position,
    pub color: Color,
    pub turn: u32,
}

/// 落子错误
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum MoveError {
    OutOfBounds,
    Occupied,
    ForbiddenMove,
    GameOver,
}

impl std::fmt::Display for MoveError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let msg = match self {
            MoveError::OutOfBounds => "坐标超出棋盘范围",
            MoveError::Occupied => "该位置已有棋子",
            MoveError::ForbiddenMove => "禁手位置，不能落子",
            MoveError::GameOver => "游戏已结束",
        };
        write!(f, "{}", msg)
    }
}

/// 落子结果
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MoveResult {
    pub position: Position,
    pub is_win: bool,
    pub is_forbidden: bool,
}

/// 游戏结果
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameResult {
    pub winner: Option<Color>,
    pub win_positions: Vec<Position>,
}

/// 游戏模式 (Tauri IPC 兼容 — 纯标签, 不含字段)
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum GameMode {
    Local,
    VsAi,
    Online,
    Replay,
}

/// 游戏配置
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameConfig {
    pub board_size: usize,
    pub use_forbidden_rules: bool,
    pub use_timer: bool,
    pub time_limit_secs: u32,
    pub ai_difficulty: u32,
    pub player_color: Color,
    pub is_server: bool,
}

impl Default for GameConfig {
    fn default() -> Self {
        Self {
            board_size: 15,
            use_forbidden_rules: true,
            use_timer: false,
            time_limit_secs: 60,
            ai_difficulty: 3,
            player_color: Color::Black,
            is_server: false,
        }
    }
}
```

文件路径: `core/src/types.rs`

- [ ] **Step 2: 更新 core/src/lib.rs 声明模块**

```rust
// Gobang core library — 纯游戏逻辑，零 GUI 依赖

pub mod types;
```

文件路径: `core/src/lib.rs`

- [ ] **Step 3: 编译验证**

```bash
cargo check -p gobang-core
```

Expected: 编译成功。

- [ ] **Step 4: 提交**

```bash
git add core/src/types.rs core/src/lib.rs
git commit -m "feat(core): 定义基础类型 — Position, Color, CellState, Move, GameConfig"
```

---

### Task 3: core/board.rs — 棋盘引擎

**Files:**
- Create: `core/src/board.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 编写测试 (TDD — RED)**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::{Color, Position, MoveError};

    #[test]
    fn test_empty_board_creation() {
        let board = Board::new(15);
        assert_eq!(board.size, 15);
        // 所有位置应为空
        for x in 0..15 {
            for y in 0..15 {
                assert_eq!(board.get(Position::new(x, y)), CellState::Empty);
            }
        }
    }

    #[test]
    fn test_place_piece() {
        let board = Board::new(15);
        let result = board.place(Position::new(7, 7), Color::Black);
        assert!(result.is_ok());
        let new_board = result.unwrap();
        assert_eq!(new_board.get(Position::new(7, 7)), CellState::Occupied(Color::Black));
    }

    #[test]
    fn test_place_on_occupied_fails() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let result = board.place(Position::new(7, 7), Color::White);
        assert_eq!(result, Err(MoveError::Occupied));
    }

    #[test]
    fn test_place_out_of_bounds_fails() {
        let board = Board::new(15);
        let result = board.place(Position::new(20, 20), Color::Black);
        assert_eq!(result, Err(MoveError::OutOfBounds));
    }

    #[test]
    fn test_win_horizontal() {
        let board = Board::new(15);
        let mut board = board;
        // 黑子连成5个水平
        for y in 3..7 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        assert!(board.check_win(Position::new(7, 7)));
    }

    #[test]
    fn test_win_vertical() {
        let board = Board::new(15);
        let mut board = board;
        for x in 3..7 {
            board = board.place(Position::new(x, 7), Color::White).unwrap();
        }
        let board = board.place(Position::new(7, 7), Color::White).unwrap();
        assert!(board.check_win(Position::new(7, 7)));
    }

    #[test]
    fn test_win_diagonal() {
        let board = Board::new(15);
        let mut board = board;
        for i in 1..5 {
            board = board.place(Position::new(3 + i, 3 + i), Color::Black).unwrap();
        }
        let board = board.place(Position::new(8, 8), Color::Black).unwrap();
        assert!(board.check_win(Position::new(8, 8)));
    }

    #[test]
    fn test_no_win_on_four() {
        let board = Board::new(15);
        let mut board = board;
        for y in 3..6 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        assert!(!board.check_win(Position::new(7, 6)));
    }

    #[test]
    fn test_undo() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();
        // 悔一步 (撤销 White 的棋)
        let board = board.undo().unwrap();
        assert_eq!(board.get(Position::new(7, 8)), CellState::Empty);
        assert_eq!(board.get(Position::new(7, 7)), CellState::Occupied(Color::Black));
    }

    #[test]
    fn test_undo_empty_history() {
        let board = Board::new(15);
        assert_eq!(board.undo(), Err(MoveError::GameOver)); // 无历史时 undo 失败
    }

    #[test]
    fn test_immutable_place() {
        let board = Board::new(15);
        let _new = board.place(Position::new(7, 7), Color::Black).unwrap();
        // 原 board 不变
        assert_eq!(board.get(Position::new(7, 7)), CellState::Empty);
    }
}
```

文件路径: `core/src/board.rs` (底部, `#[cfg(test)]` 块)

- [ ] **Step 2: 运行测试确认失败**

```bash
cargo test -p gobang-core
```

Expected: 所有测试 FAIL, `Board` 未定义。

- [ ] **Step 3: 实现 Board**

```rust
use crate::types::{CellState, Color, Move, MoveError, MoveResult, Position, MAX_BOARD_SIZE};

/// 棋盘主体 — 不可变风格, place/undo 返回新 Board
#[derive(Debug, Clone)]
pub struct Board {
    pub size: usize,
    cells: [[CellState; MAX_BOARD_SIZE]; MAX_BOARD_SIZE],
    history: Vec<Move>,
    current_turn: u32,
}

impl Board {
    /// 创建空棋盘
    pub fn new(size: usize) -> Self {
        assert!(size <= MAX_BOARD_SIZE, "棋盘尺寸不能超过 {}", MAX_BOARD_SIZE);
        Self {
            size,
            cells: [[CellState::Empty; MAX_BOARD_SIZE]; MAX_BOARD_SIZE],
            history: Vec::new(),
            current_turn: 0,
        }
    }

    /// 获取指定位置的棋子状态
    pub fn get(&self, pos: Position) -> CellState {
        if pos.x >= self.size || pos.y >= self.size {
            return CellState::Empty;
        }
        self.cells[pos.x][pos.y]
    }

    /// 落子 — 返回新 Board (不可变)
    pub fn place(&self, pos: Position, color: Color) -> Result<Board, MoveError> {
        if pos.x >= self.size || pos.y >= self.size {
            return Err(MoveError::OutOfBounds);
        }
        if self.cells[pos.x][pos.y] != CellState::Empty {
            return Err(MoveError::Occupied);
        }

        let mut new_board = self.clone();
        new_board.cells[pos.x][pos.y] = CellState::Occupied(color);
        new_board.history.push(Move {
            position: pos,
            color,
            turn: self.current_turn,
        });
        new_board.current_turn = self.current_turn + 1;
        Ok(new_board)
    }

    /// 胜负判定 — 从 pos 出发四方向扫描
    pub fn check_win(&self, pos: Position) -> bool {
        let cell = self.cells[pos.x][pos.y];
        let color = match cell {
            CellState::Occupied(c) => c,
            _ => return false,
        };

        // 四个方向: 水平(0,1), 垂直(1,0), 对角线(1,1), 反对角线(1,-1)
        let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];

        for (dx, dy) in directions {
            let mut count = 1u32;
            // 正方向
            let mut nx = pos.x as isize + dx;
            let mut ny = pos.y as isize + dy;
            while nx >= 0 && ny >= 0 && (nx as usize) < self.size && (ny as usize) < self.size {
                if self.cells[nx as usize][ny as usize] == CellState::Occupied(color) {
                    count += 1;
                    nx += dx;
                    ny += dy;
                } else {
                    break;
                }
            }
            // 反方向
            let mut nx = pos.x as isize - dx;
            let mut ny = pos.y as isize - dy;
            while nx >= 0 && ny >= 0 && (nx as usize) < self.size && (ny as usize) < self.size {
                if self.cells[nx as usize][ny as usize] == CellState::Occupied(color) {
                    count += 1;
                    nx -= dx;
                    ny -= dy;
                } else {
                    break;
                }
            }
            if count >= 5 {
                return true;
            }
        }
        false
    }

    /// 悔棋 — 撤销最近一步
    pub fn undo(&self) -> Result<Board, MoveError> {
        if self.history.is_empty() {
            return Err(MoveError::GameOver);
        }
        let mut new_board = self.clone();
        let last_move = new_board.history.pop().unwrap();
        new_board.cells[last_move.position.x][last_move.position.y] = CellState::Empty;
        new_board.current_turn = self.current_turn.saturating_sub(1);
        Ok(new_board)
    }

    /// 获取所有候选落子位 (已有棋子周围2格范围)
    pub fn get_candidate_moves(&self) -> Vec<Position> {
        let mut candidates = Vec::new();
        let range = 2isize;
        let has_stones = self.history.is_empty();

        if !has_stones {
            // 棋盘为空, 返回天元
            return vec![Position::new(self.size / 2, self.size / 2)];
        }

        for x in 0..self.size {
            for y in 0..self.size {
                if self.cells[x][y] != CellState::Empty {
                    for dx in -range..=range {
                        for dy in -range..=range {
                            let nx = x as isize + dx;
                            let ny = y as isize + dy;
                            if nx >= 0 && ny >= 0
                                && (nx as usize) < self.size
                                && (ny as usize) < self.size
                                && self.cells[nx as usize][ny as usize] == CellState::Empty
                            {
                                candidates.push(Position::new(nx as usize, ny as usize));
                            }
                        }
                    }
                }
            }
        }

        candidates.sort();
        candidates.dedup();
        candidates
    }
}

```

文件路径: `core/src/board.rs`

- [ ] **Step 4: 运行测试确认通过**

```bash
cargo test -p gobang-core
```

Expected: 所有 board 测试 PASS.

- [ ] **Step 5: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
```

- [ ] **Step 6: 最终编译 + 测试验证**

```bash
cargo test -p gobang-core
```

Expected: 全部 PASS.

- [ ] **Step 7: 提交**

```bash
git add core/src/board.rs core/src/types.rs core/src/lib.rs
git commit -m "feat(core): 棋盘引擎 — 不可变 Board, 落子/胜负/悔棋/候选位"
```

---

### Task 4: core/rules.rs — 禁手规则

**Files:**
- Create: `core/src/rules.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 编写测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_double_three_forbidden() {
        let board = Board::new(15);
        // 构造双三禁手局面: 黑子在 (7,7) 同时形成两个活三
        // 水平活三: 黑子 (7,5)(7,6) 空 (7,7) 空 (7,8)
        let board = board.place(Position::new(7, 5), Color::Black).unwrap();
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        // 斜线活三: (5,9)(6,8) 空 (7,7) 空 (8,6)
        let board = board.place(Position::new(5, 9), Color::Black).unwrap();
        let board = board.place(Position::new(6, 8), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 7), Color::Black));
    }

    #[test]
    fn test_double_four_forbidden() {
        let board = Board::new(15);
        // 构造双四禁手
        // 水平冲四: (7,3)(7,4)(7,5)(7,6) 空 (7,7)
        let board = board.place(Position::new(7, 3), Color::Black).unwrap();
        let board = board.place(Position::new(7, 4), Color::Black).unwrap();
        let board = board.place(Position::new(7, 5), Color::Black).unwrap();
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        // 垂直冲四: (3,7)(4,7)(5,7)(6,7) 空 (7,7)
        let board = board.place(Position::new(3, 7), Color::Black).unwrap();
        let board = board.place(Position::new(4, 7), Color::Black).unwrap();
        let board = board.place(Position::new(5, 7), Color::Black).unwrap();
        let board = board.place(Position::new(6, 7), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 7), Color::Black));
    }

    #[test]
    fn test_overline_forbidden() {
        let board = Board::new(15);
        // 长连禁手 (>=6)
        for y in 1..6 {
            let board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 6), Color::Black));
    }

    #[test]
    fn test_white_not_forbidden() {
        let board = Board::new(15);
        // 白棋永远不禁手
        for y in 1..6 {
            let board = board.place(Position::new(7, y), Color::White).unwrap();
        }
        let board = board.place(Position::new(7, 6), Color::White).unwrap();
        assert!(!is_forbidden(&board, Position::new(7, 6), Color::White));
    }

    #[test]
    fn test_normal_move_not_forbidden() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::Black).unwrap();
        assert!(!is_forbidden(&board, Position::new(7, 9), Color::Black));
    }
}
```

- [ ] **Step 2: 运行测试确认失败**

```bash
cargo test -p gobang-core -- rules
```

Expected: FAIL.

- [ ] **Step 3: 实现禁手检测**

```rust
use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 检测 pos 位置对 player 是否为禁手
/// 黑棋禁手: 长连(>=6)、双三、双四
/// 白棋无禁手
pub fn is_forbidden(board: &Board, pos: Position, color: Color) -> bool {
    if color == Color::White {
        return false;
    }
    is_overline(board, pos, color) || is_double_three(board, pos, color) || is_double_four(board, pos, color)
}

/// 长连检测: >=6 连
fn is_overline(board: &Board, pos: Position, color: Color) -> bool {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let mut count = 1u32;
        // 正方向
        let mut nx = pos.x as isize + dx;
        let mut ny = pos.y as isize + dy;
        while let Some(cell) = get_cell(board, nx, ny) {
            if cell == CellState::Occupied(color) { count += 1; }
            else { break; }
            nx += dx;
            ny += dy;
        }
        // 反方向
        let mut nx = pos.x as isize - dx;
        let mut ny = pos.y as isize - dy;
        while let Some(cell) = get_cell(board, nx, ny) {
            if cell == CellState::Occupied(color) { count += 1; }
            else { break; }
            nx -= dx;
            ny -= dy;
        }
        if count >= 6 {
            return true;
        }
    }
    false
}

/// 双三检测: 落子后同时产生 >=2 个活三
fn is_double_three(board: &Board, pos: Position, color: Color) -> bool {
    count_open_threes(board, pos, color) >= 2
}

/// 双四检测: 落子后同时产生 >=2 个四
fn is_double_four(board: &Board, pos: Position, color: Color) -> bool {
    count_fours(board, pos, color) >= 2
}

fn count_open_threes(board: &Board, pos: Position, color: Color) -> u32 {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    let mut count = 0u32;
    for (dx, dy) in directions {
        if is_open_three_in_direction(board, pos, color, dx, dy) {
            count += 1;
        }
    }
    count
}

fn count_fours(board: &Board, pos: Position, color: Color) -> u32 {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    let mut count = 0u32;
    for (dx, dy) in directions {
        if is_four_in_direction(board, pos, color, dx, dy) {
            count += 1;
        }
    }
    count
}

/// 活三检测: 一个方向上形成 "空-黑-黑-黑-空" (含 pos)
fn is_open_three_in_direction(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> bool {
    // 收集该方向连续同色棋子 + 两端状态
    let (cnt, start_open, end_open) = scan_direction(board, pos, color, dx, dy);
    cnt == 3 && start_open && end_open
}

/// 四检测: 冲四或活四
fn is_four_in_direction(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> bool {
    let (cnt, start_open, end_open) = scan_direction(board, pos, color, dx, dy);
    cnt == 4 // 冲四(一端开放一端堵) 或 活四(两端开放)
}

/// 扫描方向, 返回 (连续同色数, 起始端是否开放, 结束端是否开放)
fn scan_direction(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> (u32, bool, bool) {
    let mut count = 1u32;

    // 正方向
    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while let Some(cell) = get_cell(board, nx, ny) {
        if cell == CellState::Occupied(color) { count += 1; }
        else { break; }
        nx += dx;
        ny += dy;
    }
    let end_open = get_cell(board, nx, ny) == Some(CellState::Empty);

    // 反方向
    let mut nx = pos.x as isize - dx;
    let mut ny = pos.y as isize - dy;
    while let Some(cell) = get_cell(board, nx, ny) {
        if cell == CellState::Occupied(color) { count += 1; }
        else { break; }
        nx -= dx;
        ny -= dy;
    }
    let start_open = get_cell(board, nx, ny) == Some(CellState::Empty);

    (count, start_open, end_open)
}

/// 安全获取棋盘格 (边界外返回 None)
fn get_cell(board: &Board, x: isize, y: isize) -> Option<CellState> {
    if x < 0 || y < 0 || x as usize >= board.size || y as usize >= board.size {
        return None;
    }
    Some(board.get(Position::new(x as usize, y as usize)))
}
```

文件路径: `core/src/rules.rs`

- [ ] **Step 4: 运行测试确认通过**

```bash
cargo test -p gobang-core -- rules
```

Expected: 所有 rules 测试 PASS.

- [ ] **Step 5: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
pub mod rules;
```

- [ ] **Step 6: 提交**

```bash
git add core/src/rules.rs core/src/lib.rs
git commit -m "feat(core): 禁手规则 — 长连/双三/双四检测"
```

---

### Task 5: core/ai/evaluate.rs — 棋形评分

**Files:**
- Create: `core/src/ai/mod.rs`
- Create: `core/src/ai/evaluate.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 创建 ai/mod.rs — AiEngine trait**

```rust
use crate::board::Board;
use crate::types::{Color, Position};

/// AI 引擎统一接口
pub trait AiEngine: Send + Sync {
    /// 返回 AI 的最佳落子位置, 无位置返回 None
    fn best_move(&self, board: &Board, color: Color) -> Option<Position>;
}

pub mod evaluate;
pub mod search;
```

文件路径: `core/src/ai/mod.rs`

- [ ] **Step 2: 编写 evaluate 测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_evaluate_empty_board() {
        let board = Board::new(15);
        let score = evaluate_board(&board, Color::Black);
        // 空棋盘得分应为 0 (双方都没有棋)
        assert_eq!(score, 0.0);
    }

    #[test]
    fn test_five_in_a_row_high_score() {
        let board = Board::new(15);
        let mut board = board;
        for y in 5..10 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let score = evaluate_board(&board, Color::Black);
        assert!(score > 10000.0); // 连五得分极高
    }
}
```

- [ ] **Step 3: 运行测试确认失败**

```bash
cargo test -p gobang-core -- evaluate
```

Expected: FAIL.

- [ ] **Step 4: 实现棋形评分**

```rust
use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 棋形分数 (参考 v1 C 版评分逻辑)
const FIVE: f64 = 100000.0;
const OPEN_FOUR: f64 = 10000.0;
const RUSH_FOUR: f64 = 5000.0;
const OPEN_THREE: f64 = 1000.0;
const SLEEP_THREE: f64 = 500.0;
const OPEN_TWO: f64 = 100.0;
const SLEEP_TWO: f64 = 50.0;
const OPEN_ONE: f64 = 10.0;

/// 评估整个棋盘对 player 的得分
/// 返回 (player 得分, 对手得分)
pub fn evaluate_board(board: &Board, player: Color) -> f64 {
    let player_score = evaluate_player(board, player);
    let opponent_score = evaluate_player(board, player.opponent());
    player_score - opponent_score
}

fn evaluate_player(board: &Board, color: Color) -> f64 {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    let mut total = 0.0f64;
    let size = board.size;

    for x in 0..size {
        for y in 0..size {
            if board.get(Position::new(x, y)) != CellState::Occupied(color) {
                continue;
            }
            for &(dx, dy) in &directions {
                let (count, start_open, end_open) =
                    scan_pattern(board, Position::new(x, y), color, dx, dy);
                total += score_pattern(count, start_open, end_open);
            }
        }
    }
    total
}

/// 从 pos 向 (dx,dy) 方向扫描, 只扫描正方向(避免重复计数)
fn scan_pattern(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> (u32, bool, bool) {
    let mut count = 1u32;

    // 正方向
    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while in_bounds(board, nx, ny) && board.get(Position::new(nx as usize, ny as usize)) == CellState::Occupied(color) {
        count += 1;
        nx += dx;
        ny += dy;
    }
    let end_open = in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty;

    // 反方向 (起点)
    let sx = pos.x as isize - dx;
    let sy = pos.y as isize - dy;
    let start_open = in_bounds(board, sx, sy)
        && board.get(Position::new(sx as usize, sy as usize)) == CellState::Empty;

    // 只在这个方向第一次遇到连续段时计数 (避免重复)
    // 检查反方向是不是同色: 如果是, 说明不是起点, 不计分
    if in_bounds(board, sx, sy) && board.get(Position::new(sx as usize, sy as usize)) == CellState::Occupied(color) {
        return (0, false, false); // 不是起点, 不计分
    }

    (count, start_open, end_open)
}

fn score_pattern(count: u32, start_open: bool, end_open: bool) -> f64 {
    let open_count = start_open as u32 + end_open as u32;
    match (count, open_count) {
        (5, _) => FIVE,
        (4, 2) => OPEN_FOUR,
        (4, 1) => RUSH_FOUR,
        (3, 2) => OPEN_THREE,
        (3, 1) => SLEEP_THREE,
        (2, 2) => OPEN_TWO,
        (2, 1) => SLEEP_TWO,
        (1, 2) => OPEN_ONE,
        _ => 0.0,
    }
}

fn in_bounds(board: &Board, x: isize, y: isize) -> bool {
    x >= 0 && y >= 0 && (x as usize) < board.size && (y as usize) < board.size
}
```

文件路径: `core/src/ai/evaluate.rs`

- [ ] **Step 5: 运行测试确认通过**

```bash
cargo test -p gobang-core -- evaluate
```

Expected: PASS.

- [ ] **Step 6: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
pub mod rules;
pub mod ai;
```

- [ ] **Step 7: 提交**

```bash
git add core/src/ai/ core/src/lib.rs
git commit -m "feat(core): AI 棋形评分模块 — 连五/活四/冲四/活三等棋形打分"
```

---

### Task 6: core/ai/search.rs — Alpha-Beta 搜索

**Files:**
- Create: `core/src/ai/search.rs`

- [ ] **Step 1: 编写测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use crate::ai::AiEngine;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_ai_returns_center_on_empty_board() {
        let board = Board::new(15);
        let ai = AlphaBetaAi::new(1);
        let mv = ai.best_move(&board, Color::Black);
        assert!(mv.is_some());
        let pos = mv.unwrap();
        // 天元附近
        assert!(pos.x >= 6 && pos.x <= 8);
        assert!(pos.y >= 6 && pos.y <= 8);
    }

    #[test]
    fn test_ai_blocks_four() {
        let board = Board::new(15);
        // 白棋冲四: (7,3)(7,4)(7,5)(7,6) — AI(黑) 应堵 (7,2) 或 (7,7)
        let board = board.place(Position::new(7, 3), Color::White).unwrap();
        let board = board.place(Position::new(7, 4), Color::White).unwrap();
        let board = board.place(Position::new(7, 5), Color::White).unwrap();
        let board = board.place(Position::new(7, 6), Color::White).unwrap();
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black).unwrap();
        // 应该堵在端点
        assert!(
            (mv.x == 7 && mv.y == 2) || (mv.x == 7 && mv.y == 7),
            "AI should block four, got ({},{})", mv.x, mv.y
        );
    }

    #[test]
    fn test_ai_takes_win() {
        let board = Board::new(15);
        // 黑棋可连五: (7,3)(7,4)(7,5)(7,6) — AI(黑) 应该下 (7,7)
        let board = board.place(Position::new(7, 3), Color::Black).unwrap();
        let board = board.place(Position::new(7, 4), Color::Black).unwrap();
        let board = board.place(Position::new(7, 5), Color::Black).unwrap();
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black).unwrap();
        assert_eq!(mv, Position::new(7, 7));
    }
}
```

文件路径: `core/src/ai/search.rs` (底部)

- [ ] **Step 2: 运行测试确认失败**

```bash
cargo test -p gobang-core -- search
```

Expected: FAIL.

- [ ] **Step 3: 实现 AlphaBetaAi**

```rust
use crate::ai::evaluate::evaluate_board;
use crate::ai::AiEngine;
use crate::board::Board;
use crate::types::{Color, Position, MoveResult};

/// Alpha-Beta AI 引擎
pub struct AlphaBetaAi {
    depth: usize,
    defense_coefficient: f64,
}

impl AlphaBetaAi {
    pub fn new(depth: usize) -> Self {
        Self {
            depth,
            defense_coefficient: 1.2,
        }
    }

    pub fn with_defense(mut self, coeff: f64) -> Self {
        self.defense_coefficient = coeff;
        self
    }
}

impl AiEngine for AlphaBetaAi {
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return None;
        }

        let mut best_pos = None;
        let mut best_score = f64::NEG_INFINITY;

        for &pos in &candidates {
            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    return Some(pos); // 直接赢, 立即返回
                }
                let score = -self.negamax(
                    &new_board,
                    self.depth - 1,
                    f64::NEG_INFINITY,
                    f64::INFINITY,
                    color.opponent(),
                );
                if score > best_score {
                    best_score = score;
                    best_pos = Some(pos);
                }
            }
        }

        best_pos
    }
}

impl AlphaBetaAi {
    fn negamax(
        &self,
        board: &Board,
        depth: usize,
        mut alpha: f64,
        beta: f64,
        color: Color,
    ) -> f64 {
        if depth == 0 {
            return evaluate_board(board, color);
        }

        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return evaluate_board(board, color);
        }

        // 启发式排序: 按候选位评分降序
        let mut scored: Vec<(Position, f64)> = candidates
            .into_iter()
            .filter_map(|pos| {
                board.place(pos, color).ok().map(|b| {
                    if b.check_win(pos) {
                        (pos, f64::INFINITY)
                    } else {
                        let s = evaluate_board(&b, color);
                        (pos, s)
                    }
                })
            })
            .collect();
        scored.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));

        let mut max_val = f64::NEG_INFINITY;
        for (pos, _) in scored {
            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    return f64::INFINITY; // 必胜
                }
                let val = -self.negamax(
                    &new_board,
                    depth - 1,
                    -beta,
                    -alpha,
                    color.opponent(),
                );
                if val > max_val {
                    max_val = val;
                }
                if val > alpha {
                    alpha = val;
                }
                if alpha >= beta {
                    break; // 剪枝
                }
            }
        }
        max_val
    }
}
```

文件路径: `core/src/ai/search.rs`

- [ ] **Step 4: 运行测试确认通过**

```bash
cargo test -p gobang-core -- search
```

Expected: 所有测试 PASS.

- [ ] **Step 5: 提交**

```bash
git add core/src/ai/search.rs
git commit -m "feat(core): AI Alpha-Beta 搜索 — Negamax + 剪枝 + 启发式排序"
```

---

### Task 7: core/record.rs — 棋谱记录

**Files:**
- Create: `core/src/record.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 编写测试**

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_save_and_load_record() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();

        let record = GameRecord::from_board(&board, "Human", "AI-Lv3", Some(Color::Black));
        let json = serde_json::to_string_pretty(&record).unwrap();

        let loaded: GameRecord = serde_json::from_str(&json).unwrap();
        assert_eq!(loaded.moves.len(), 2);
        assert_eq!(loaded.moves[0].position, Position::new(7, 7));
    }

    #[test]
    fn test_replay_board() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();

        let record = GameRecord::from_board(&board, "Human", "AI", None);
        let replayed = record.to_replay_board().unwrap();

        // 重建后棋盘应和原始一致
        assert_eq!(replayed.get(Position::new(7, 7)), crate::types::CellState::Occupied(Color::Black));
        assert_eq!(replayed.get(Position::new(7, 8)), crate::types::CellState::Occupied(Color::White));
    }
}
```

- [ ] **Step 2: 运行测试确认失败**

```bash
cargo test -p gobang-core -- record
```

Expected: FAIL.

- [ ] **Step 3: 实现棋谱模块**

```rust
use serde::{Deserialize, Serialize};
use crate::board::Board;
use crate::types::{Color, Move, Position};

/// 对局棋谱
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameRecord {
    pub version: String,
    pub date: String,
    pub board_size: usize,
    pub black_player: String,
    pub white_player: String,
    pub winner: Option<String>,
    pub moves: Vec<RecordMove>,
}

/// 棋谱中的一步
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RecordMove {
    pub x: usize,
    pub y: usize,
    pub color: String,
    pub turn: u32,
}

impl GameRecord {
    pub fn new(board_size: usize, black: &str, white: &str) -> Self {
        Self {
            version: "2.0".to_string(),
            date: chrono_now(),
            board_size,
            black_player: black.to_string(),
            white_player: white.to_string(),
            winner: None,
            moves: Vec::new(),
        }
    }

    pub fn from_board(board: &Board, black: &str, white: &str, winner: Option<Color>) -> Self {
        let winner_str = winner.map(|c| match c {
            Color::Black => black.to_string(),
            Color::White => white.to_string(),
        });
        let moves = board.history().iter().map(|m| RecordMove {
            x: m.position.x,
            y: m.position.y,
            color: match m.color { Color::Black => "Black".into(), Color::White => "White".into() },
            turn: m.turn,
        }).collect();

        Self {
            version: "2.0".to_string(),
            date: chrono_now(),
            board_size: board.size,
            black_player: black.to_string(),
            white_player: white.to_string(),
            winner: winner_str,
            moves,
        }
    }

    /// 从棋谱重建最终棋盘
    pub fn to_replay_board(&self) -> Result<Board, String> {
        let mut board = Board::new(self.board_size);
        for m in &self.moves {
            let color = match m.color.as_str() {
                "Black" => Color::Black,
                "White" => Color::White,
                _ => return Err(format!("未知颜色: {}", m.color)),
            };
            board = board.place(Position::new(m.x, m.y), color)
                .map_err(|e| e.to_string())?;
        }
        Ok(board)
    }
}

fn chrono_now() -> String {
    // 避免引入 chrono crate, 用简单格式
    use std::time::SystemTime;
    if let Ok(dur) = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
        let secs = dur.as_secs();
        let days = secs / 86400;
        // 简化: 天数从1970算起, 不作为真实日期
        format!("day-{}", days)
    } else {
        "unknown".to_string()
    }
}
```

文件路径: `core/src/record.rs`

- [ ] **Step 4: Board 需要暴露 history**

在 `core/src/board.rs` 的 `impl Board` 中添加:

```rust
/// 获取落子历史 (用于棋谱)
pub fn history(&self) -> &[Move] {
    &self.history
}
```

- [ ] **Step 5: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
pub mod rules;
pub mod ai;
pub mod record;
```

- [ ] **Step 6: 运行测试确认通过**

```bash
cargo test -p gobang-core
```

Expected: 全部 PASS.

- [ ] **Step 7: 提交**

```bash
git add core/src/record.rs core/src/board.rs core/src/lib.rs
git commit -m "feat(core): 棋谱记录 — JSON 序列化/反序列化 + 复盘重建"
```

---

### Task 8: core/network.rs — renet 网络对战

**Files:**
- Create: `core/src/network.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 实现网络模块 (无独立测试, 依赖 renet runtime)**

```rust
use serde::{Deserialize, Serialize};
use crate::types::Position;

/// 游戏网络消息
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum GameMessage {
    Move { x: usize, y: usize, turn: u32 },
    Undo { steps: u32 },
    Resign,
    Chat(String),
    Heartbeat,
}

/// 网络连接角色
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NetworkRole {
    Server,
    Client,
}

/// 网络会话配置
#[derive(Debug, Clone)]
pub struct NetworkConfig {
    pub role: NetworkRole,
    pub bind_port: u16,
    pub remote_addr: String,
    pub remote_port: u16,
}

/// 网络会话状态
#[derive(Debug, Clone)]
pub struct NetworkSession {
    pub role: NetworkRole,
    pub is_connected: bool,
    pub config: NetworkConfig,
    pending_messages: Vec<GameMessage>,
}

impl NetworkSession {
    pub fn new(config: NetworkConfig) -> Self {
        Self {
            role: config.role,
            is_connected: false,
            config,
            pending_messages: Vec::new(),
        }
    }

    /// 发送消息 (实际 renet 集成在 gui 层处理)
    pub fn enqueue_message(&mut self, msg: GameMessage) {
        self.pending_messages.push(msg);
    }

    /// 取出待发送的消息
    pub fn drain_messages(&mut self) -> Vec<GameMessage> {
        std::mem::take(&mut self.pending_messages)
    }

    pub fn set_connected(&mut self, connected: bool) {
        self.is_connected = connected;
    }
}
```

文件路径: `core/src/network.rs`

- [ ] **Step 2: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
pub mod rules;
pub mod ai;
pub mod record;
pub mod network;
```

- [ ] **Step 3: 编译验证**

```bash
cargo check -p gobang-core
```

- [ ] **Step 4: 提交**

```bash
git add core/src/network.rs core/src/lib.rs
git commit -m "feat(core): 网络模块 — GameMessage 协议定义 + NetworkSession"
```

---

### Task 9: core/llm.rs — 大模型 AI

**Files:**
- Create: `core/src/llm.rs`
- Modify: `core/src/lib.rs`

- [ ] **Step 1: 实现 LLM AI (实现 AiEngine trait)**

```rust
use crate::ai::AiEngine;
use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 大模型 AI — 通过 HTTP API 调用
pub struct LlmAi {
    endpoint: String,
    api_key: String,
    model: String,
}

impl LlmAi {
    pub fn new(endpoint: &str, api_key: &str, model: &str) -> Self {
        Self {
            endpoint: endpoint.to_string(),
            api_key: api_key.to_string(),
            model: model.to_string(),
        }
    }

    /// 将棋盘序列化为 prompt
    pub fn board_to_prompt(board: &Board, color: Color) -> String {
        let mut s = String::from("你是一位五子棋高手。当前棋盘状态(0=空,1=黑,2=白):\n");
        for x in 0..board.size {
            for y in 0..board.size {
                let ch = match board.get(Position::new(x, y)) {
                    CellState::Empty => '0',
                    CellState::Occupied(Color::Black) => '1',
                    CellState::Occupied(Color::White) => '2',
                };
                s.push(ch);
                s.push(' ');
            }
            s.push('\n');
        }
        let color_str = match color {
            Color::Black => "黑棋(1)",
            Color::White => "白棋(2)",
        };
        s.push_str(&format!("\n你是{}, 请返回最佳落子坐标 (格式: x,y)", color_str));
        s
    }

    /// 解析 LLM 响应中的坐标
    pub fn parse_response(response: &str) -> Option<Position> {
        // 尝试匹配 "x,y" 格式
        for part in response.split([' ', '\n', '\r', '(', ')', '[', ']']) {
            let trimmed = part.trim();
            if let Some(comma_pos) = trimmed.find(',') {
                let x_str = &trimmed[..comma_pos];
                let y_str = &trimmed[comma_pos + 1..];
                if let (Ok(x), Ok(y)) = (x_str.parse::<usize>(), y_str.parse::<usize>()) {
                    return Some(Position::new(x, y));
                }
            }
        }
        None
    }
}

impl AiEngine for LlmAi {
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        // 同步 HTTP 请求 (在 Tauri 异步命令中通过 spawn_blocking 调用)
        let prompt = Self::board_to_prompt(board, color);
        let client = reqwest::blocking::Client::new();
        let body = serde_json::json!({
            "model": self.model,
            "messages": [
                {"role": "user", "content": prompt}
            ],
            "max_tokens": 50,
            "temperature": 0.3
        });

        let resp = client
            .post(&self.endpoint)
            .header("Authorization", format!("Bearer {}", self.api_key))
            .header("Content-Type", "application/json")
            .json(&body)
            .send()
            .ok()?;

        let json: serde_json::Value = resp.json().ok()?;
        let content = json["choices"][0]["message"]["content"].as_str()?;
        Self::parse_response(content)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_coordinate() {
        assert_eq!(LlmAi::parse_response("7,8"), Some(Position::new(7, 8)));
        assert_eq!(LlmAi::parse_response("(7, 8)"), Some(Position::new(7, 8)));
        assert_eq!(LlmAi::parse_response("坐标是 10,5"), Some(Position::new(10, 5)));
        assert_eq!(LlmAi::parse_response("no coordinate"), None);
    }

    #[test]
    fn test_board_to_prompt() {
        let board = Board::new(15);
        let prompt = LlmAi::board_to_prompt(&board, Color::Black);
        assert!(prompt.contains("黑棋(1)"));
        assert!(prompt.contains("0 0 0"));
    }
}
```

文件路径: `core/src/llm.rs`

- [ ] **Step 2: 添加 reqwest 依赖到 core/Cargo.toml**

确认 `core/Cargo.toml` 已包含:
```toml
reqwest = { version = "0.12", features = ["json", "blocking"] }
```

- [ ] **Step 3: 更新 lib.rs**

```rust
pub mod types;
pub mod board;
pub mod rules;
pub mod ai;
pub mod record;
pub mod network;
pub mod llm;
```

- [ ] **Step 4: 运行测试**

```bash
cargo test -p gobang-core -- llm
```

Expected: PASS (parse 测试, 不需要网络).

- [ ] **Step 5: 提交**

```bash
git add core/src/llm.rs core/src/lib.rs
git commit -m "feat(core): LLM AI — OpenAI 兼容 API 调用 + prompt/parse"
```

---

### Task 10: 前端脚手架 — Tauri + React + Vite + TypeScript 初始化

**Files:**
- Create: `gui/Cargo.toml`
- Create: `gui/build.rs`
- Create: `gui/tauri.conf.json`
- Create: `gui/src/main.rs`
- Create: `gui/src/lib.rs`
- Create: `package.json`
- Create: `tsconfig.json`
- Create: `tsconfig.node.json`
- Create: `vite.config.ts`
- Create: `index.html`
- Create: `src/main.tsx`
- Create: `src/App.tsx`
- Create: `src/index.css`
- Create: `src/App.css`

- [ ] **Step 1: 创建 gui/Cargo.toml**

```toml
[package]
name = "gobang-gui"
version.workspace = true
edition.workspace = true
license.workspace = true
authors.workspace = true
repository.workspace = true

[build-dependencies]
tauri-build = { version = "2", features = [] }

[dependencies]
tauri = { version = "2", features = [] }
tauri-plugin-opener = "2"
serde = { version = "1", features = ["derive"] }
serde_json = "1"
gobang-core = { path = "../core" }
```

文件路径: `gui/Cargo.toml`

- [ ] **Step 2: 创建 gui/build.rs**

```rust
fn main() {
    tauri_build::build()
}
```

- [ ] **Step 3: 创建 gui/tauri.conf.json**

从 PathEditor `gui/tauri.conf.json` 获取模板, 修改:
- `productName` → "Gobang"
- `identifier` → "com.liuhangyu.gobang"
- `title` → "五子棋 v2.0"
- 窗口默认大小 900x700

```json
{
  "$schema": "https://raw.githubusercontent.com/nicknisi/tauri-config-schema/main/tauri.conf.json",
  "productName": "Gobang",
  "version": "2.0.0",
  "identifier": "com.liuhangyu.gobang",
  "build": {
    "frontendDist": "../dist",
    "devUrl": "http://localhost:1420",
    "beforeBuildCommand": "npm run build",
    "beforeDevCommand": "npm run dev"
  },
  "app": {
    "windows": [
      {
        "title": "五子棋 v2.0",
        "width": 900,
        "height": 700,
        "resizable": true,
        "center": true
      }
    ],
    "security": {
      "csp": null
    }
  },
  "bundle": {
    "active": true,
    "targets": "all",
    "icon": [
      "icons/32x32.png",
      "icons/128x128.png",
      "icons/128x128@2x.png",
      "icons/icon.icns",
      "icons/icon.ico"
    ]
  }
}
```

文件路径: `gui/tauri.conf.json`

- [ ] **Step 4: 创建 gui/src/main.rs**

```rust
// Prevents additional console window on Windows in release
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

fn main() {
    gobang_gui::run()
}
```

- [ ] **Step 5: 创建 gui/src/lib.rs (空壳)**

```rust
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
```

- [ ] **Step 6: 创建 package.json**

从 PathEditor `package.json` 获取模板, 替换:
- `name` → "gobang"
- `productName` → "Gobang"
- `version` → "2.0.0"

```json
{
  "name": "gobang",
  "private": true,
  "version": "2.0.0",
  "type": "module",
  "scripts": {
    "dev": "vite",
    "build": "tsc -b && vite build",
    "preview": "vite preview",
    "test": "vitest",
    "test:watch": "vitest --watch"
  },
  "dependencies": {
    "react": "^19.0.0",
    "react-dom": "^19.0.0",
    "zustand": "^5.0.0",
    "i18next": "^24.0.0",
    "react-i18next": "^15.0.0",
    "@tauri-apps/api": "^2.0.0",
    "@tauri-apps/plugin-opener": "^2.0.0"
  },
  "devDependencies": {
    "@types/react": "^19.0.0",
    "@types/react-dom": "^19.0.0",
    "@vitejs/plugin-react": "^4.4.0",
    "typescript": "~5.7.0",
    "vite": "^6.0.0",
    "vitest": "^3.0.0",
    "@tauri-apps/cli": "^2.0.0"
  }
}
```

文件路径: `package.json`

- [ ] **Step 7: 创建 tsconfig.json**

从 PathEditor 复制 `tsconfig.json`。

- [ ] **Step 8: 创建 vite.config.ts**

从 PathEditor 复制 `vite.config.ts`, 修改端口以适应 Tauri devUrl。

- [ ] **Step 9: 创建 index.html**

```html
<!DOCTYPE html>
<html lang="zh-CN">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>五子棋 v2.0</title>
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.tsx"></script>
  </body>
</html>
```

- [ ] **Step 10: 创建 src/main.tsx, src/App.tsx, src/index.css, src/App.css (最小可用)**

```tsx
// src/main.tsx
import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App';
import './index.css';

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
```

```tsx
// src/App.tsx
function App() {
  return (
    <div className="app">
      <h1>五子棋 v2.0</h1>
    </div>
  );
}

export default App;
```

- [ ] **Step 11: npm install + 验证编译**

```bash
npm install
npx tauri dev
```

Expected: 窗口打开, 显示 "五子棋 v2.0"。

- [ ] **Step 12: 提交**

```bash
git add gui/ package.json tsconfig.json tsconfig.node.json vite.config.ts index.html src/
git commit -m "feat: Tauri + React + Vite + TypeScript 前端脚手架"
```

---

### Task 11: gui/commands.rs + AppState — Tauri IPC 桥接

**Files:**
- Create: `gui/src/commands.rs`
- Modify: `gui/src/lib.rs`

- [ ] **Step 1: 实现 commands.rs**

```rust
use gobang_core::ai::{AlphaBetaAi, AiEngine, evaluate, search};
use gobang_core::board::Board;
use gobang_core::types::*;
use gobang_core::rules;
use std::sync::Mutex;
use tauri::State;

/// 应用全局状态
pub struct AppState {
    pub board: Mutex<Option<Board>>,
    pub game_mode: Mutex<GameMode>,
    pub config: Mutex<GameConfig>,
    pub ai_engine: Mutex<Option<AlphaBetaAi>>,
    pub current_color: Mutex<Color>,
    pub game_over: Mutex<bool>,
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            board: Mutex::new(None),
            game_mode: Mutex::new(GameMode::Local),
            config: Mutex::new(GameConfig::default()),
            ai_engine: Mutex::new(None),
            current_color: Mutex::new(Color::Black),
            game_over: Mutex::new(true),
        }
    }
}

#[tauri::command]
fn new_game(mode: GameMode, config: GameConfig, state: State<AppState>) -> Result<(), String> {
    let board = Board::new(config.board_size);
    *state.board.lock().map_err(|e| e.to_string())? = Some(board);
    *state.game_mode.lock().map_err(|e| e.to_string())? = mode;
    *state.config.lock().map_err(|e| e.to_string())? = config.clone();
    *state.current_color.lock().map_err(|e| e.to_string())? = config.player_color;
    *state.game_over.lock().map_err(|e| e.to_string())? = false;

    // 初始化 AI (如果是人机模式)
    if mode == GameMode::VsAi {
        let ai = AlphaBetaAi::new(config.ai_difficulty as usize);
        *state.ai_engine.lock().map_err(|e| e.to_string())? = Some(ai);
    }

    Ok(())
}

#[tauri::command]
fn place_piece(x: usize, y: usize, state: State<AppState>) -> Result<MoveResult, String> {
    let mut game_over = state.game_over.lock().map_err(|e| e.to_string())?;
    if *game_over {
        return Err("游戏已结束".into());
    }

    let mut board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref().ok_or("游戏未开始")?;
    let color = *state.current_color.lock().map_err(|e| e.to_string())?;
    let config = state.config.lock().map_err(|e| e.to_string())?;

    let pos = Position::new(x, y);

    // 禁手检查
    if config.use_forbidden_rules && rules::is_forbidden(board, pos, color) {
        return Err("禁手位置，不能落子".into());
    }

    let new_board = board.place(pos, color).map_err(|e| e.to_string())?;
    let is_win = new_board.check_win(pos);

    if is_win {
        *game_over = true;
    }

    *state.current_color.lock().map_err(|e| e.to_string())? = color.opponent();
    *board_opt = Some(new_board);

    Ok(MoveResult {
        position: pos,
        is_win,
        is_forbidden: false,
    })
}

#[tauri::command]
fn undo(steps: u32, state: State<AppState>) -> Result<(), String> {
    let mut board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let mut board = board_opt.clone().ok_or("游戏未开始")?;

    for _ in 0..steps * 2 {
        // 每步撤销双方各一手
        board = board.undo().map_err(|e| e.to_string())?;
    }

    *board_opt = Some(board);
    Ok(())
}

#[tauri::command]
fn get_board(state: State<AppState>) -> Result<Vec<Vec<i32>>, String> {
    let board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref().ok_or("游戏未开始")?;

    let mut result = vec![vec![0i32; board.size]; board.size];
    for x in 0..board.size {
        for y in 0..board.size {
            result[x][y] = match board.get(Position::new(x, y)) {
                CellState::Empty => 0,
                CellState::Occupied(Color::Black) => 1,
                CellState::Occupied(Color::White) => 2,
            };
        }
    }
    Ok(result)
}

#[tauri::command]
fn ai_move(state: State<AppState>) -> Result<Option<(usize, usize)>, String> {
    let board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref().ok_or("游戏未开始")?;
    let color = *state.current_color.lock().map_err(|e| e.to_string())?;
    let ai = state.ai_engine.lock().map_err(|e| e.to_string())?;
    let ai = ai.as_ref().ok_or("AI 未初始化")?;

    Ok(ai.best_move(board, color).map(|p| (p.x, p.y)))
}

#[tauri::command]
fn get_game_state(state: State<AppState>) -> Result<serde_json::Value, String> {
    let board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let color = *state.current_color.lock().map_err(|e| e.to_string())?;
    let game_over = *state.game_over.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref();

    let cells: Vec<Vec<i32>> = board.map(|b| {
        (0..b.size).map(|x| {
            (0..b.size).map(move |y| {
                match b.get(Position::new(x, y)) {
                    CellState::Empty => 0,
                    CellState::Occupied(Color::Black) => 1,
                    CellState::Occupied(Color::White) => 2,
                }
            }).collect()
        }).collect()
    }).unwrap_or_default();

    Ok(serde_json::json!({
        "board": cells,
        "current_color": match color { Color::Black => "Black", Color::White => "White" },
        "game_over": game_over,
    }))
}
```

文件路径: `gui/src/commands.rs`

- [ ] **Step 2: 更新 gui/src/lib.rs 注册命令**

```rust
mod commands;

use commands::AppState;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .manage(AppState::default())
        .invoke_handler(tauri::generate_handler![
            commands::new_game,
            commands::place_piece,
            commands::undo,
            commands::get_board,
            commands::ai_move,
            commands::get_game_state,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
```

文件路径: `gui/src/lib.rs`

- [ ] **Step 3: 编译验证**

```bash
cargo check
```

- [ ] **Step 4: 提交**

```bash
git add gui/
git commit -m "feat(gui): Tauri IPC 命令 — new_game/place_piece/undo/ai_move/get_board/get_game_state"
```

---

### Task 12: 前端核心 — types.ts + constants.ts + i18n + store

**Files:**
- Create: `src/core/types.ts`
- Create: `src/core/constants.ts`
- Create: `src/i18n/index.ts`
- Create: `src/i18n/zh-CN.json`
- Create: `src/i18n/en.json`
- Create: `src/store/gameStore.ts`

- [ ] **Step 1: 创建 src/core/types.ts**

```typescript
export type Color = 'Black' | 'White';

export interface Position {
  x: number;
  y: number;
}

export type CellState = 0 | 1 | 2; // 0=Empty, 1=Black, 2=White

export type GameStatus = 'waiting' | 'playing' | 'ai_thinking' | 'game_over';

export type GameModeType = 'Local' | 'VsAi' | 'Online' | 'Replay';

export interface GameConfig {
  boardSize: number;
  useForbiddenRules: boolean;
  useTimer: boolean;
  timeLimitSecs: number;
  aiDifficulty: number;
  playerColor: Color;
  isServer: boolean;
}

export interface MoveResult {
  position: Position;
  is_win: boolean;
  is_forbidden: boolean;
}

export interface Move {
  position: Position;
  color: Color;
  turn: number;
}
```

- [ ] **Step 2: 创建 src/core/constants.ts**

```typescript
export const DEFAULT_BOARD_SIZE = 15;
export const MIN_BOARD_SIZE = 9;
export const MAX_BOARD_SIZE = 19;

export const CELL_COLORS: Record<number, string> = {
  0: 'transparent',
  1: '#1a1a1a', // 黑子
  2: '#f5f5f5', // 白子
};
```

- [ ] **Step 3: 创建 src/i18n/zh-CN.json**

```json
{
  "app": {
    "title": "五子棋 v2.0"
  },
  "menu": {
    "local_game": "本地双人",
    "ai_game": "人机对战",
    "online_game": "网络对战",
    "load_replay": "加载棋谱",
    "settings": "设置"
  },
  "game": {
    "black_turn": "黑棋回合",
    "white_turn": "白棋回合",
    "black_win": "黑棋获胜!",
    "white_win": "白棋获胜!",
    "draw": "平局",
    "ai_thinking": "AI 思考中...",
    "undo": "悔棋",
    "resign": "认输",
    "save": "保存棋谱",
    "new_game": "新游戏",
    "waiting_opponent": "等待对手加入...",
    "your_turn": "你的回合",
    "opponent_turn": "对手回合"
  },
  "replay": {
    "play": "播放",
    "pause": "暂停",
    "next": "下一步",
    "prev": "上一步",
    "step": "第 {{current}}/{{total}} 步"
  },
  "settings": {
    "board_size": "棋盘大小",
    "forbidden_rules": "禁手规则",
    "timer": "计时器",
    "time_limit": "时间限制(秒)",
    "difficulty": "AI 难度",
    "language": "语言"
  }
}
```

- [ ] **Step 4: 创建 src/i18n/en.json**

```json
{
  "app": {
    "title": "Gobang v2.0"
  },
  "menu": {
    "local_game": "Local 2-Player",
    "ai_game": "VS AI",
    "online_game": "Online",
    "load_replay": "Load Replay",
    "settings": "Settings"
  },
  "game": {
    "black_turn": "Black's Turn",
    "white_turn": "White's Turn",
    "black_win": "Black Wins!",
    "white_win": "White Wins!",
    "draw": "Draw",
    "ai_thinking": "AI Thinking...",
    "undo": "Undo",
    "resign": "Resign",
    "save": "Save Record",
    "new_game": "New Game",
    "waiting_opponent": "Waiting for Opponent...",
    "your_turn": "Your Turn",
    "opponent_turn": "Opponent's Turn"
  },
  "replay": {
    "play": "Play",
    "pause": "Pause",
    "next": "Next",
    "prev": "Prev",
    "step": "Step {{current}}/{{total}}"
  },
  "settings": {
    "board_size": "Board Size",
    "forbidden_rules": "Forbidden Rules",
    "timer": "Timer",
    "time_limit": "Time Limit (s)",
    "difficulty": "AI Difficulty",
    "language": "Language"
  }
}
```

- [ ] **Step 5: 创建 src/i18n/index.ts**

```typescript
import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import zhCN from './zh-CN.json';
import en from './en.json';

i18n
  .use(initReactI18next)
  .init({
    resources: {
      'zh-CN': { translation: zhCN },
      en: { translation: en },
    },
    lng: 'zh-CN',
    fallbackLng: 'zh-CN',
    interpolation: { escapeValue: false },
  });

export default i18n;
```

- [ ] **Step 6: 创建 src/store/gameStore.ts**

```typescript
import { create } from 'zustand';
import { invoke } from '@tauri-apps/api/core';
import type { CellState, Color, GameConfig, GameModeType, GameStatus, Move, MoveResult } from '../core/types';

interface GameState {
  mode: GameModeType;
  board: CellState[][];
  boardSize: number;
  currentColor: Color;
  status: GameStatus;
  winner: Color | null;
  moves: Move[];
  config: GameConfig;
  isSaving: boolean;

  // Actions
  startGame: (mode: GameModeType, config: GameConfig) => Promise<void>;
  placePiece: (x: number, y: number) => Promise<MoveResult>;
  undo: (steps?: number) => Promise<void>;
  aiMove: () => Promise<void>;
  refreshBoard: () => Promise<void>;
  loadReplayBoard: (board: CellState[][], moves: Move[]) => void;
}

export const useGameStore = create<GameState>((set, get) => ({
  mode: 'Local',
  board: [],
  boardSize: 15,
  currentColor: 'Black',
  status: 'waiting',
  winner: null,
  moves: [],
  config: {
    boardSize: 15,
    useForbiddenRules: true,
    useTimer: false,
    timeLimitSecs: 60,
    aiDifficulty: 3,
    playerColor: 'Black',
    isServer: false,
  },
  isSaving: false,

  startGame: async (mode, config) => {
    await invoke('new_game', { mode, config });
    set({
      mode,
      config,
      boardSize: config.boardSize,
      status: mode === 'VsAi' && config.playerColor === 'White' ? 'ai_thinking' : 'playing',
      currentColor: 'Black',
      winner: null,
      moves: [],
    });
    await get().refreshBoard();
  },

  placePiece: async (x, y) => {
    const result: MoveResult = await invoke('place_piece', { x, y });
    await get().refreshBoard();
    if (result.is_win) {
      set({ status: 'game_over' });
    }
    return result;
  },

  undo: async (steps = 1) => {
    await invoke('undo', { steps });
    await get().refreshBoard();
  },

  aiMove: async () => {
    set({ status: 'ai_thinking' });
    const pos: [number, number] | null = await invoke('ai_move');
    if (pos) {
      const result = await get().placePiece(pos[0], pos[1]);
      if (!result.is_win) {
        set({ status: 'playing' });
      }
    } else {
      set({ status: 'playing' });
    }
  },

  refreshBoard: async () => {
    const state: { board: CellState[][]; current_color: string; game_over: boolean } =
      await invoke('get_game_state');
    set({
      board: state.board,
      currentColor: state.current_color as Color,
      status: state.game_over ? 'game_over' : get().status === 'ai_thinking' ? 'ai_thinking' : 'playing',
    });
  },

  loadReplayBoard: (board, moves) => {
    set({ board, moves, mode: 'Replay', status: 'playing' });
  },
}));
```

- [ ] **Step 7: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 8: 提交**

```bash
git add src/core/ src/i18n/ src/store/
git commit -m "feat(frontend): 类型定义 + i18n 中英双语 + Zustand store"
```

---

### Task 13: 前端 — BoardCanvas + board-renderer

**Files:**
- Create: `src/components/board/board-renderer.ts`
- Create: `src/components/board/BoardCanvas.tsx`

- [ ] **Step 1: 创建 board-renderer.ts (纯函数, 零 React 依赖)**

```typescript
import type { CellState, Position } from '../../core/types';

export interface RenderConfig {
  cellSize: number;
  padding: number;
  boardSize: number;
}

export function computeBoardDimensions(boardSize: number, canvasWidth: number, canvasHeight: number): RenderConfig {
  const maxBoardPixelSize = Math.min(canvasWidth, canvasHeight) * 0.85;
  const cellSize = Math.floor(maxBoardPixelSize / (boardSize - 1));
  const actualBoardPixelSize = cellSize * (boardSize - 1);
  const padding = Math.floor((Math.min(canvasWidth, canvasHeight) - actualBoardPixelSize) / 2);
  return { cellSize, padding, boardSize };
}

export function canvasToBoard(
  canvasX: number,
  canvasY: number,
  cfg: RenderConfig
): Position | null {
  const col = Math.round((canvasX - cfg.padding) / cfg.cellSize);
  const row = Math.round((canvasY - cfg.padding) / cfg.cellSize);
  if (col < 0 || col >= cfg.boardSize || row < 0 || row >= cfg.boardSize) return null;
  return { x: row, y: col };
}

export function boardToCanvas(pos: Position, cfg: RenderConfig): { x: number; y: number } {
  return {
    x: cfg.padding + pos.y * cfg.cellSize,
    y: cfg.padding + pos.x * cfg.cellSize,
  };
}

export function renderBoard(
  ctx: CanvasRenderingContext2D,
  board: CellState[][],
  cfg: RenderConfig,
  lastMove: Position | null
): void {
  const { cellSize, padding, boardSize } = cfg;
  const width = padding * 2 + (boardSize - 1) * cellSize;
  const height = width;

  // 背景 (木纹色)
  ctx.fillStyle = '#DEB887';
  ctx.fillRect(0, 0, width + padding, height + padding);

  // 棋盘区域
  ctx.fillStyle = '#F5DEB3';
  ctx.fillRect(padding - 10, padding - 10, (boardSize - 1) * cellSize + 20, (boardSize - 1) * cellSize + 20);

  // 网格线
  ctx.strokeStyle = '#8B7355';
  ctx.lineWidth = 1;
  for (let i = 0; i < boardSize; i++) {
    // 水平线
    ctx.beginPath();
    ctx.moveTo(padding, padding + i * cellSize);
    ctx.lineTo(padding + (boardSize - 1) * cellSize, padding + i * cellSize);
    ctx.stroke();
    // 垂直线
    ctx.beginPath();
    ctx.moveTo(padding + i * cellSize, padding);
    ctx.lineTo(padding + i * cellSize, padding + (boardSize - 1) * cellSize);
    ctx.stroke();
  }

  // 星位 (天元和四角星)
  const starPoints = [
    [3, 3], [3, 7], [3, 11],
    [7, 3], [7, 7], [7, 11],
    [11, 3], [11, 7], [11, 11],
  ];
  ctx.fillStyle = '#8B7355';
  for (const [r, c] of starPoints) {
    if (r < boardSize && c < boardSize) {
      const { x, y } = boardToCanvas({ x: r, y: c }, cfg);
      ctx.beginPath();
      ctx.arc(x, y, 3, 0, Math.PI * 2);
      ctx.fill();
    }
  }

  // 棋子
  for (let x = 0; x < boardSize; x++) {
    for (let y = 0; y < boardSize; y++) {
      if (board[x]?.[y] === 0) continue;
      const { x: cx, y: cy } = boardToCanvas({ x, y }, cfg);
      const radius = cellSize * 0.43;

      if (board[x][y] === 1) {
        // 黑子渐变
        const gradient = ctx.createRadialGradient(cx - 2, cy - 2, 1, cx, cy, radius);
        gradient.addColorStop(0, '#4a4a4a');
        gradient.addColorStop(1, '#1a1a1a');
        ctx.fillStyle = gradient;
      } else {
        // 白子渐变
        const gradient = ctx.createRadialGradient(cx - 2, cy - 2, 1, cx, cy, radius);
        gradient.addColorStop(0, '#ffffff');
        gradient.addColorStop(1, '#d0d0d0');
        ctx.fillStyle = gradient;
      }

      ctx.beginPath();
      ctx.arc(cx, cy, radius, 0, Math.PI * 2);
      ctx.fill();

      // 白子边框
      if (board[x][y] === 2) {
        ctx.strokeStyle = '#b0b0b0';
        ctx.lineWidth = 1;
        ctx.stroke();
      }
    }
  }

  // 最后一手高亮
  if (lastMove) {
    const { x, y } = boardToCanvas(lastMove, cfg);
    ctx.strokeStyle = '#ff4444';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(x, y, cellSize * 0.2, 0, Math.PI * 2);
    ctx.stroke();
  }
}
```

- [ ] **Step 2: 创建 BoardCanvas.tsx**

```tsx
import { useEffect, useRef, useCallback } from 'react';
import { useGameStore } from '../../store/gameStore';
import {
  computeBoardDimensions,
  canvasToBoard,
  renderBoard,
  type RenderConfig,
} from './board-renderer';
import type { Position } from '../../core/types';

export default function BoardCanvas() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const board = useGameStore((s) => s.board);
  const boardSize = useGameStore((s) => s.boardSize);
  const status = useGameStore((s) => s.status);
  const mode = useGameStore((s) => s.mode);
  const placePiece = useGameStore((s) => s.placePiece);
  const aiMove = useGameStore((s) => s.aiMove);
  const moves = useGameStore((s) => s.moves);

  const lastMove = moves.length > 0 ? moves[moves.length - 1].position : null;

  const render = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    ctx.scale(dpr, dpr);

    const cfg = computeBoardDimensions(boardSize, rect.width, rect.height);
    renderBoard(ctx, board, cfg, lastMove);
  }, [board, boardSize, lastMove]);

  useEffect(() => {
    render();
    const handleResize = () => render();
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, [render]);

  const handleClick = useCallback(
    (e: React.MouseEvent<HTMLCanvasElement>) => {
      if (status !== 'playing') return;
      if (mode === 'VsAi' && moves.length % 2 === 1) return; // AI 回合, 不响应点击
      if (mode === 'Replay') return; // 回放模式不落子

      const canvas = canvasRef.current;
      if (!canvas) return;
      const rect = canvas.getBoundingClientRect();
      const cfg = computeBoardDimensions(boardSize, rect.width, rect.height);
      const pos = canvasToBoard(e.clientX - rect.left, e.clientY - rect.top, cfg);
      if (!pos) return;

      placePiece(pos.x, pos.y).then((result) => {
        if (!result.is_win && mode === 'VsAi') {
          setTimeout(() => aiMove(), 100);
        }
      });
    },
    [status, mode, boardSize, moves.length, placePiece, aiMove]
  );

  return (
    <canvas
      ref={canvasRef}
      onClick={handleClick}
      style={{
        width: '100%',
        height: '100%',
        cursor: status === 'playing' && mode !== 'Replay' ? 'pointer' : 'default',
      }}
    />
  );
}
```

- [ ] **Step 3: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add src/components/board/
git commit -m "feat(frontend): Canvas 棋盘渲染 — 木纹风格, 棋子渐变, 最后一手高亮"
```

---

### Task 14: 前端 — 菜单组件

**Files:**
- Create: `src/components/menu/MainMenu.tsx`
- Create: `src/components/menu/LocalGameSetup.tsx`
- Create: `src/components/menu/AiGameSetup.tsx`
- Create: `src/components/menu/OnlineSetup.tsx`
- Create: `src/components/menu/LoadReplay.tsx`

- [ ] **Step 1: 创建 MainMenu.tsx**

```tsx
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import LocalGameSetup from './LocalGameSetup';
import AiGameSetup from './AiGameSetup';
import OnlineSetup from './OnlineSetup';
import LoadReplay from './LoadReplay';

type View = 'main' | 'local' | 'ai' | 'online' | 'replay';

interface Props {
  onGameStart: () => void;
}

export default function MainMenu({ onGameStart }: Props) {
  const { t } = useTranslation();
  const [view, setView] = useState<View>('main');

  if (view === 'local') return <LocalGameSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'ai') return <AiGameSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'online') return <OnlineSetup onBack={() => setView('main')} onStart={onGameStart} />;
  if (view === 'replay') return <LoadReplay onBack={() => setView('main')} onStart={onGameStart} />;

  return (
    <div className="main-menu">
      <h1 className="menu-title">{t('app.title')}</h1>
      <div className="menu-buttons">
        <button onClick={() => setView('local')}>{t('menu.local_game')}</button>
        <button onClick={() => setView('ai')}>{t('menu.ai_game')}</button>
        <button onClick={() => setView('online')}>{t('menu.online_game')}</button>
        <button onClick={() => setView('replay')}>{t('menu.load_replay')}</button>
      </div>
    </div>
  );
}
```

- [ ] **Step 2: 创建 LocalGameSetup.tsx**

```tsx
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import type { GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function LocalGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize: 15,
      useForbiddenRules: true,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: 3,
      playerColor: 'Black',
      isServer: false,
    };
    await startGame('Local', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.local_game')}</h2>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>返回</button>
      </div>
    </div>
  );
}
```

- [ ] **Step 3: 创建 AiGameSetup.tsx**

```tsx
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import type { Color, GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function AiGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [difficulty, setDifficulty] = useState(3);
  const [playerColor, setPlayerColor] = useState<Color>('Black');
  const [useForbidden, setUseForbidden] = useState(true);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize: 15,
      useForbiddenRules: useForbidden,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: difficulty,
      playerColor,
      isServer: false,
    };
    await startGame('VsAi', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.ai_game')}</h2>
      <label>
        {t('settings.difficulty')}:
        <select value={difficulty} onChange={(e) => setDifficulty(Number(e.target.value))}>
          {[1, 2, 3, 4, 5].map((d) => (
            <option key={d} value={d}>{d}</option>
          ))}
        </select>
      </label>
      <label>
        先手:
        <select value={playerColor} onChange={(e) => setPlayerColor(e.target.value as Color)}>
          <option value="Black">黑棋 (先手)</option>
          <option value="White">白棋 (后手)</option>
        </select>
      </label>
      <label>
        <input type="checkbox" checked={useForbidden} onChange={(e) => setUseForbidden(e.target.checked)} />
        {t('settings.forbidden_rules')}
      </label>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>返回</button>
      </div>
    </div>
  );
}
```

- [ ] **Step 4: 创建 OnlineSetup.tsx 和 LoadReplay.tsx**

```tsx
// OnlineSetup.tsx
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { useState } from 'react';

interface Props { onBack: () => void; onStart: () => void; }

export default function OnlineSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [ip, setIp] = useState('');

  const handleHost = async () => {
    await startGame('Online', { boardSize: 15, useForbiddenRules: true, useTimer: false, timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'Black', isServer: true });
    onStart();
  };

  const handleJoin = async () => {
    await startGame('Online', { boardSize: 15, useForbiddenRules: true, useTimer: false, timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'Black', isServer: false });
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.online_game')}</h2>
      <button onClick={handleHost}>创建房间</button>
      <div>
        <input value={ip} onChange={(e) => setIp(e.target.value)} placeholder="IP:端口" />
        <button onClick={handleJoin} disabled={!ip}>加入房间</button>
      </div>
      <button onClick={onBack}>返回</button>
    </div>
  );
}
```

```tsx
// LoadReplay.tsx
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { useRef } from 'react';
import type { Move } from '../../core/types';

interface Props { onBack: () => void; onStart: () => void; }

export default function LoadReplay({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const loadReplayBoard = useGameStore((s) => s.loadReplayBoard);
  const fileRef = useRef<HTMLInputElement>(null);

  const handleFile = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = () => {
      try {
        const json = JSON.parse(reader.result as string);
        const board: (0 | 1 | 2)[][] = Array.from({ length: json.board_size }, () =>
          Array(json.board_size).fill(0)
        );
        const moves: Move[] = [];
        for (const m of json.moves) {
          board[m.x][m.y] = m.color === 'Black' ? 1 : 2;
          moves.push({ position: { x: m.x, y: m.y }, color: m.color, turn: m.turn });
        }
        loadReplayBoard(board, moves);
        onStart();
      } catch {
        alert('无效的棋谱文件');
      }
    };
    reader.readAsText(file);
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.load_replay')}</h2>
      <input ref={fileRef} type="file" accept=".json" onChange={handleFile} />
      <button onClick={onBack}>返回</button>
    </div>
  );
}
```

- [ ] **Step 5: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 6: 提交**

```bash
git add src/components/menu/
git commit -m "feat(frontend): 菜单组件 — 主菜单/本地双人/AI设置/网络/加载棋谱"
```

---

### Task 15: 前端 — 对局 + 回放视图

**Files:**
- Create: `src/components/game/GameView.tsx`
- Create: `src/components/game/GameInfo.tsx`
- Create: `src/components/game/GameControls.tsx`
- Create: `src/components/game/TimerDisplay.tsx`
- Create: `src/components/replay/ReplayView.tsx`
- Create: `src/components/replay/StepSlider.tsx`
- Create: `src/components/replay/ReplayControls.tsx`
- Create: `src/hooks/useGame.ts`
- Create: `src/hooks/useTimer.ts`

- [ ] **Step 1: 创建 GameInfo.tsx**

```tsx
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';

export default function GameInfo() {
  const { t } = useTranslation();
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const winner = useGameStore((s) => s.winner);

  let text = '';
  if (status === 'game_over' && winner) {
    text = winner === 'Black' ? t('game.black_win') : t('game.white_win');
  } else if (status === 'ai_thinking') {
    text = t('game.ai_thinking');
  } else if (status === 'playing') {
    text = currentColor === 'Black' ? t('game.black_turn') : t('game.white_turn');
  }

  return <div className="game-info">{text}</div>;
}
```

- [ ] **Step 2: 创建 GameControls.tsx**

```tsx
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';

interface Props {
  onBackToMenu: () => void;
}

export default function GameControls({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const undo = useGameStore((s) => s.undo);
  const mode = useGameStore((s) => s.mode);
  const status = useGameStore((s) => s.status);

  const handleUndo = () => {
    if (mode === 'VsAi') undo(1); // 人机模式悔棋悔双方各一手
    else undo(1);
  };

  return (
    <div className="game-controls">
      <button onClick={handleUndo} disabled={status === 'game_over'}>
        {t('game.undo')}
      </button>
      <button onClick={onBackToMenu}>{t('game.new_game')}</button>
    </div>
  );
}
```

- [ ] **Step 3: 创建 TimerDisplay.tsx**

```tsx
import { useState, useEffect } from 'react';
import { useGameStore } from '../../store/gameStore';

export default function TimerDisplay() {
  const config = useGameStore((s) => s.config);
  const currentColor = useGameStore((s) => s.currentColor);
  const status = useGameStore((s) => s.status);
  const [time, setTime] = useState(config.timeLimitSecs);

  useEffect(() => {
    if (!config.useTimer || status !== 'playing') return;
    setTime(config.timeLimitSecs);
    const timer = setInterval(() => {
      setTime((t) => {
        if (t <= 1) { clearInterval(timer); return 0; }
        return t - 1;
      });
    }, 1000);
    return () => clearInterval(timer);
  }, [currentColor, config.useTimer, config.timeLimitSecs, status]);

  if (!config.useTimer) return null;

  return (
    <div className={`timer-display ${time <= 10 ? 'timer-warning' : ''}`}>
      {Math.floor(time / 60)}:{(time % 60).toString().padStart(2, '0')}
    </div>
  );
}
```

- [ ] **Step 4: 创建 GameView.tsx**

```tsx
import BoardCanvas from '../board/BoardCanvas';
import GameInfo from './GameInfo';
import GameControls from './GameControls';
import TimerDisplay from './TimerDisplay';

interface Props {
  onBackToMenu: () => void;
}

export default function GameView({ onBackToMenu }: Props) {
  return (
    <div className="game-view">
      <GameInfo />
      <div className="board-container">
        <BoardCanvas />
      </div>
      <TimerDisplay />
      <GameControls onBackToMenu={onBackToMenu} />
    </div>
  );
}
```

- [ ] **Step 5: 创建 ReplayView.tsx + StepSlider.tsx + ReplayControls.tsx**

```tsx
// StepSlider.tsx
interface Props {
  current: number;
  total: number;
  onChange: (step: number) => void;
}

export default function StepSlider({ current, total, onChange }: Props) {
  return (
    <input
      type="range"
      min={0}
      max={total}
      value={current}
      onChange={(e) => onChange(Number(e.target.value))}
      className="step-slider"
    />
  );
}
```

```tsx
// ReplayControls.tsx
import { useTranslation } from 'react-i18next';

interface Props {
  isPlaying: boolean;
  onTogglePlay: () => void;
  onPrev: () => void;
  onNext: () => void;
}

export default function ReplayControls({ isPlaying, onTogglePlay, onPrev, onNext }: Props) {
  const { t } = useTranslation();
  return (
    <div className="replay-controls">
      <button onClick={onPrev}>{t('replay.prev')}</button>
      <button onClick={onTogglePlay}>{isPlaying ? t('replay.pause') : t('replay.play')}</button>
      <button onClick={onNext}>{t('replay.next')}</button>
    </div>
  );
}
```

```tsx
// ReplayView.tsx
import { useState, useEffect, useCallback } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import BoardCanvas from '../board/BoardCanvas';
import StepSlider from './StepSlider';
import ReplayControls from './ReplayControls';

interface Props {
  onBackToMenu: () => void;
}

export default function ReplayView({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const moves = useGameStore((s) => s.moves);
  const [step, setStep] = useState(moves.length);
  const [isPlaying, setIsPlaying] = useState(false);

  useEffect(() => {
    if (!isPlaying) return;
    if (step >= moves.length) {
      setIsPlaying(false);
      return;
    }
    const timer = setInterval(() => setStep((s) => s + 1), 500);
    return () => clearInterval(timer);
  }, [isPlaying, step, moves.length]);

  return (
    <div className="replay-view">
      <div className="board-container">
        <BoardCanvas />
      </div>
      <StepSlider current={step} total={moves.length} onChange={setStep} />
      <div>{t('replay.step', { current: step, total: moves.length })}</div>
      <ReplayControls
        isPlaying={isPlaying}
        onTogglePlay={() => setIsPlaying(!isPlaying)}
        onPrev={() => setStep(Math.max(0, step - 1))}
        onNext={() => setStep(Math.min(moves.length, step + 1))}
      />
      <button onClick={onBackToMenu}>返回菜单</button>
    </div>
  );
}
```

- [ ] **Step 6: 创建 hooks**

```typescript
// src/hooks/useGame.ts
import { useCallback } from 'react';
import { useGameStore } from '../store/gameStore';
import type { GameConfig, GameModeType } from '../core/types';

export function useGame() {
  const store = useGameStore();

  const startGame = useCallback(async (mode: GameModeType, config: GameConfig) => {
    await store.startGame(mode, config);
  }, [store]);

  return { ...store, startGame };
}
```

```typescript
// src/hooks/useTimer.ts
import { useState, useEffect } from 'react';

export function useTimer(seconds: number, active: boolean, onTimeout: () => void) {
  const [time, setTime] = useState(seconds);

  useEffect(() => {
    if (!active) return;
    setTime(seconds);
    const timer = setInterval(() => {
      setTime((t) => {
        if (t <= 1) { clearInterval(timer); onTimeout(); return 0; }
        return t - 1;
      });
    }, 1000);
    return () => clearInterval(timer);
  }, [active, seconds, onTimeout]);

  return time;
}
```

- [ ] **Step 7: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 8: 提交**

```bash
git add src/components/game/ src/components/replay/ src/hooks/
git commit -m "feat(frontend): 对局视图 + 回放视图 + 计时器 hook"
```

---

### Task 16: 前端 — App.tsx 集成 + 样式

**Files:**
- Modify: `src/App.tsx`
- Modify: `src/App.css`
- Modify: `src/index.css`

- [ ] **Step 1: 更新 App.tsx — 路由集成**

```tsx
import { useState } from 'react';
import { useGameStore } from './store/gameStore';
import MainMenu from './components/menu/MainMenu';
import GameView from './components/game/GameView';
import ReplayView from './components/replay/ReplayView';
import i18n from './i18n';
import './App.css';

type Page = 'menu' | 'game' | 'replay';

function App() {
  const [page, setPage] = useState<Page>('menu');
  const mode = useGameStore((s) => s.mode);

  const handleGameStart = () => {
    setPage('game');
  };

  const handleReplayStart = () => {
    setPage('replay');
  };

  const handleBackToMenu = () => {
    setPage('menu');
  };

  if (page === 'game') {
    return <GameView onBackToMenu={handleBackToMenu} />;
  }

  if (page === 'replay') {
    return <ReplayView onBackToMenu={handleBackToMenu} />;
  }

  return <MainMenu onGameStart={handleGameStart} />;
}

export default App;
```

- [ ] **Step 2: 创建 App.css — 木纹风格样式**

```css
/* 全局变量 */
:root {
  --bg-primary: #3C2415;
  --bg-secondary: #F5DEB3;
  --text-primary: #F5DEB3;
  --text-secondary: #3C2415;
  --accent: #8B4513;
  --accent-hover: #A0522D;
  --button-bg: #DEB887;
  --button-hover: #D2B48C;
  --border: #8B7355;
}

* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Microsoft YaHei', sans-serif;
  background-color: var(--bg-primary);
  color: var(--text-primary);
}

.app {
  width: 100vw;
  height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
}

/* 主菜单 */
.main-menu {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100vh;
  gap: 40px;
}

.menu-title {
  font-size: 42px;
  color: var(--text-primary);
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
}

.menu-buttons {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.menu-buttons button {
  width: 240px;
  padding: 14px 28px;
  font-size: 18px;
  border: 2px solid var(--border);
  border-radius: 8px;
  background: var(--button-bg);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all 0.2s;
}

.menu-buttons button:hover {
  background: var(--button-hover);
  transform: scale(1.03);
}

/* 设置面板 */
.setup-panel {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100vh;
  gap: 24px;
}

.setup-panel h2 {
  font-size: 28px;
}

.setup-panel label {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 16px;
}

.setup-panel select, .setup-panel input {
  padding: 6px 12px;
  border: 1px solid var(--border);
  border-radius: 4px;
  background: var(--bg-secondary);
  color: var(--text-secondary);
  font-size: 14px;
}

.setup-actions {
  display: flex;
  gap: 12px;
  margin-top: 16px;
}

button {
  padding: 10px 24px;
  font-size: 16px;
  border: 2px solid var(--border);
  border-radius: 6px;
  background: var(--button-bg);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all 0.2s;
}

button:hover {
  background: var(--button-hover);
}

button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

/* 对局视图 */
.game-view {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100vw;
  height: 100vh;
  padding: 12px;
  gap: 8px;
}

.board-container {
  flex: 1;
  width: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden;
}

.game-info {
  font-size: 20px;
  font-weight: bold;
  padding: 8px;
}

.game-controls {
  display: flex;
  gap: 12px;
  padding: 8px;
}

.timer-display {
  font-size: 24px;
  font-family: monospace;
}

.timer-warning {
  color: #ff4444;
  animation: blink 0.5s infinite alternate;
}

@keyframes blink {
  from { opacity: 1; }
  to { opacity: 0.3; }
}

/* 回放视图 */
.replay-view {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100vw;
  height: 100vh;
  padding: 12px;
  gap: 8px;
}

.step-slider {
  width: 80%;
  accent-color: var(--accent);
}

.replay-controls {
  display: flex;
  gap: 12px;
}
```

- [ ] **Step 3: 更新 index.css**

```css
html, body, #root {
  margin: 0;
  padding: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
}
```

- [ ] **Step 4: 验证**

```bash
npx tauri dev
```

手动测试: 点击菜单项进入各页面, 验证 Canvas 渲染正常, 木纹风格生效。

- [ ] **Step 5: 提交**

```bash
git add src/App.tsx src/App.css src/index.css
git commit -m "feat(frontend): App 路由集成 + 木纹风格 CSS"
```

---

### Task 17: 开源文件 + README

**Files:**
- Create: `LICENSE`
- Create: `CHANGELOG.md`
- Create: `CODE_OF_CONDUCT.md`
- Create: `CONTRIBUTING.md`
- Create: `SECURITY.md`
- Rewrite: `README.md`

- [ ] **Step 1: 从 PathEditor 复制开源文件并替换项目名**

所有文件以 PathEditor 对应文件为模板:
- `LICENSE` — 从 `D:\Code\doing_exercises\programs\PathEditor\LICENSE` 复制 (MIT)
- `CODE_OF_CONDUCT.md` — 从 PathEditor 复制 (行为准则通用, 无需修改)
- `CONTRIBUTING.md` — 从 PathEditor 复制, 替换:
  - PathEditor → Gobang
  - patheditor → gobang
  - 删除 CLI 相关内容 (无 cli crate)
  - Rust + Node.js 版本要求保持一致
- `SECURITY.md` — 从 PathEditor 复制, 替换:
  - PathEditor → Gobang
  - `~/.patheditor/` → `~/.gobang/`
  - 版本改为 v2.x
- `CHANGELOG.md` — 新建, 写入:

```markdown
# Changelog

## 2.0.0 (2026-05-30)

### Added
- Rust + Tauri 2.x + React 19 + TypeScript strict 全重写
- Cargo workspace 两 crate 架构 (core + gui)
- Canvas 木纹风格棋盘渲染
- 中英双语界面 (i18next)
- Alpha-Beta 剪枝 AI 引擎 (5 级难度)
- LLM 大模型 AI (OpenAI 兼容 API)
- renet 网络对战 (纯 Rust ENet 协议)
- JSON 棋谱记录与回放
- Zustand 状态管理

### Changed
- 从 C + IUP + CMake 迁移到 Rust + Tauri + Vite
- 棋谱格式从二进制改为 JSON
- 网络协议从 ENet C 库改为 renet 纯 Rust 实现
- AI 从评分制升级为 Alpha-Beta 搜索
```

- [ ] **Step 2: 重写 README.md**

```markdown
# Gobang (五子棋) v2.0

Rust + Tauri 2.x + React 19 构建的五子棋桌面应用。

## 功能

- 本地双人对战
- 人机对战 (Alpha-Beta 剪枝 AI, 5 级难度)
- 网络对战 (renet P2P)
- LLM 大模型 AI
- 棋谱记录与回放 (JSON)
- 禁手规则
- 中/英双语

## 开发

### 环境要求

- Node.js 22+
- Rust 1.95+ (stable-x86_64-pc-windows-gnu)
- MinGW-w64
- Windows 10+

### 命令

```bash
npm install
npx tauri dev     # 开发模式
npx tauri build   # 生产构建
cargo test        # Rust 测试
npm test          # 前端测试
cargo clippy -- -D warnings  # Lint
```

## 架构

```
core/   # Rust 游戏核心库 (零 Tauri 依赖)
gui/    # Tauri 桌面应用 (薄命令层)
src/    # React 前端 (TypeScript strict)
```

## 许可

MIT
```

- [ ] **Step 3: 提交**

```bash
git add LICENSE CHANGELOG.md CODE_OF_CONDUCT.md CONTRIBUTING.md SECURITY.md README.md
git commit -m "docs: 开源文件 — LICENSE/CHANGELOG/CODE_OF_CONDUCT/CONTRIBUTING/SECURITY/README"
```

---

### 最终验证

```bash
# Rust 全量检查
cargo check
cargo clippy -- -D warnings
cargo test

# 前端检查
npx tsc -b
npm test

# 完整构建
npx tauri build
```
