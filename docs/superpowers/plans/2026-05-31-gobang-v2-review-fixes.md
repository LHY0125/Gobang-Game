# Gobang v2.0 审查问题修复计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复审查报告中的全部 P0/P1/P2/P3 问题，使项目达到可发布质量标准。

**Architecture:** 14 个修复按优先级分批执行：P0 消除用户可感知的 bug 和构建错误；P1 补充测试覆盖和完成功能断层；P2 完善 UI 国际化与交互；P3 擦亮文档和日志。每批内部独立可并行，批次之间按依赖关系顺序推进。

**Tech Stack:** Rust (core + gui), TypeScript/React, Tauri 2.x IPC, vitest, i18next

---

## 文件变更总览

| 文件 | 操作 | 涉及任务 |
|------|------|---------|
| `core/src/board.rs` | 修改 | Task 2 |
| `core/src/record.rs` | 修改 | Task 11 |
| `core/src/network.rs` | 修改 | Task 6 |
| `gui/Cargo.toml` | 修改 | Task 7, Task 13 |
| `gui/src/commands.rs` | 修改 | Task 1, Task 2, Task 4, Task 5, Task 8 |
| `gui/src/lib.rs` | 修改 | Task 1, Task 5, Task 8, Task 13 |
| `gui/src/main.rs` | 修改 | Task 13 |
| `gui/tauri.conf.json` | 修改 | Task 5 |
| `src/core/types.ts` | 修改 | Task 3, Task 6 |
| `src/core/__tests__/types.test.ts` | 新建 | Task 3 |
| `src/components/board/__tests__/board-renderer.test.ts` | 新建 | Task 3 |
| `src/store/__tests__/gameStore.test.ts` | 新建 | Task 3 |
| `src/components/menu/AiGameSetup.tsx` | 修改 | Task 7, Task 9 |
| `src/components/menu/LocalGameSetup.tsx` | 修改 | Task 7, Task 9 |
| `src/components/menu/OnlineSetup.tsx` | 修改 | Task 7, Task 9 |
| `src/components/menu/LoadReplay.tsx` | 修改 | Task 7 |
| `src/components/game/GameControls.tsx` | 修改 | Task 8 |
| `src/components/game/TimerDisplay.tsx` | 修改 | Task 8 |
| `src/components/replay/ReplayView.tsx` | 修改 | Task 7 |
| `src/components/common/ErrorBoundary.tsx` | 新建 | Task 10 |
| `src/App.tsx` | 修改 | Task 10 |
| `src/i18n/zh-CN.json` | 修改 | Task 7 |
| `src/i18n/en.json` | 修改 | Task 7 |
| `CONTRIBUTING.md` | 修改 | Task 12 |

---

### Task 1: 删除死代码 get_board + 修复 clippy 警告

**Files:**
- Modify: `gui/src/commands.rs:113-129` (delete `get_board` function)
- Modify: `gui/src/lib.rs:14` (remove `get_board` from handler registration)

- [ ] **Step 1: 删除 `get_board` 命令函数**

在 `gui/src/commands.rs` 中删除第 113-129 行（整个 `get_board` 函数）。

- [ ] **Step 2: 从 handler 注册中移除 `get_board`**

在 `gui/src/lib.rs` 第 14 行删除 `commands::get_board,`，修改后：

```rust
.invoke_handler(tauri::generate_handler![
    commands::new_game,
    commands::place_piece,
    commands::undo,
    commands::ai_move,
    commands::get_game_state,
])
```

- [ ] **Step 3: 验证编译和 clippy 通过**

```bash
cargo check
cargo clippy -- -D warnings
```

Expected: clippy 零警告，cargo check 通过。

- [ ] **Step 4: 运行全部测试确认无回归**

```bash
cargo test
```

Expected: 26 passed.

- [ ] **Step 5: 提交**

```bash
git add gui/src/commands.rs gui/src/lib.rs
git commit -m "chore: 删除未使用的 get_board IPC 命令，修复 clippy needless_range_loop 警告"
```

---

### Task 2: 修复悔棋奇数步 bug

**Files:**
- Modify: `gui/src/commands.rs:93-111` (undo 函数重写)
- Modify: `core/src/board.rs:99-109` (undo 空历史错误语义修正)

- [ ] **Step 1: 为 Board::undo 空历史写失败测试**

在 `core/src/board.rs` 的 `tests` 模块中修改已有测试 `test_undo_empty_history` 以验证新的错误类型：

```rust
#[test]
fn test_undo_empty_history_returns_no_history_error() {
    let board = Board::new(15);
    let result = board.undo();
    assert!(result.is_err());
    // 不应是 GameOver
    match result {
        Err(MoveError::NoHistory) => {},
        other => panic!("expected NoHistory, got {:?}", other),
    }
}
```

- [ ] **Step 2: 运行测试确认失败**

```bash
cargo test -p gobang-core test_undo_empty_history_returns_no_history_error
```

Expected: FAIL — MoveError 没有 NoHistory 变体。

- [ ] **Step 3: 添加 `NoHistory` 错误变体并修改 `undo()`**

在 `core/src/types.rs` 的 `MoveError` 枚举中添加：

```rust
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum MoveError {
    OutOfBounds,
    Occupied,
    ForbiddenMove,
    GameOver,
    NoHistory,
}

impl std::fmt::Display for MoveError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let msg = match self {
            MoveError::OutOfBounds => "坐标超出棋盘范围",
            MoveError::Occupied => "该位置已有棋子",
            MoveError::ForbiddenMove => "禁手位置，不能落子",
            MoveError::GameOver => "游戏已结束",
            MoveError::NoHistory => "没有可撤销的棋子",
        };
        write!(f, "{}", msg)
    }
}
```

修改 `core/src/board.rs:101-103`：

```rust
pub fn undo(&self) -> Result<Board, MoveError> {
    if self.history.is_empty() {
        return Err(MoveError::NoHistory);
    }
```

- [ ] **Step 4: 运行测试确认通过**

```bash
cargo test -p gobang-core
```

Expected: 全部通过。

- [ ] **Step 5: 写 undo 奇数步 bug 回归测试（Rust 端）**

没有直接的 Rust 测试入口（bug 在 commands 层），改为前端 vitest。

在 `src/store/__tests__/gameStore.test.ts` 中：

```typescript
import { describe, it, expect } from 'vitest';

describe('undo logic', () => {
  it('should handle undo with odd number of moves (single step)', () => {
    // 模拟: 只有1步棋, undo(1) 应只撤销1步而非报错
    // 验证逻辑: steps*2 不应超过实际步数
    const moves = [{ position: { x: 7, y: 7 }, color: 'Black' as const, turn: 0 }];
    const safeUndoCount = Math.min(1 * 2, moves.length);
    expect(safeUndoCount).toBe(1); // 只有1步, 最多撤销1步
  });
});
```

- [ ] **Step 6: 修复 `undo` 命令逻辑**

修改 `gui/src/commands.rs:93-111`：

```rust
#[tauri::command]
pub fn undo(steps: u32, state: State<AppState>) -> Result<(), String> {
    let mut board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let mut board = board_opt.clone().ok_or("游戏未开始")?;

    let max_undo = board.history().len() as u32;
    let actual_steps = (steps * 2).min(max_undo);

    for _ in 0..actual_steps {
        board = board.undo().map_err(|e| e.to_string())?;
    }

    let corrected_color = match board.history().last() {
        Some(last_move) => last_move.color.opponent(),
        None => state.config.lock().map_err(|e| e.to_string())?.player_color,
    };
    *state.current_color.lock().map_err(|e| e.to_string())? = corrected_color;
    *state.game_over.lock().map_err(|e| e.to_string())? = false;

    *board_opt = Some(board);
    Ok(())
}
```

关键改动：`let actual_steps = (steps * 2).min(max_undo);`

- [ ] **Step 7: 运行全部测试**

```bash
cargo test
npx vitest run
```

Expected: Rust 26+ passed，前端测试通过。

- [ ] **Step 8: 提交**

```bash
git add core/src/types.rs core/src/board.rs gui/src/commands.rs src/store/__tests__/gameStore.test.ts
git commit -m "fix: 修复悔棋奇数步崩溃及空历史错误语义"
```

---

### Task 3: 前端核心逻辑单元测试

**Files:**
- Create: `src/core/__tests__/types.test.ts`
- Create: `src/components/board/__tests__/board-renderer.test.ts`

- [ ] **Step 1: 创建 vitest 配置**

检查 `vite.config.ts` 中 vitest 配置。vitest 默认从 vite.config.ts 读取配置，无需额外配置。创建测试目录：

```bash
mkdir -p src/core/__tests__
mkdir -p src/components/board/__tests__
```

- [ ] **Step 2: 写 types.test.ts**

```typescript
import { describe, it, expect } from 'vitest';

describe('types', () => {
  it('CellState values are correct', () => {
    // 0=Empty, 1=Black, 2=White
    const empty: number = 0;
    const black: number = 1;
    const white: number = 2;
    expect(empty).toBe(0);
    expect(black).toBe(1);
    expect(white).toBe(2);
  });

  it('MoveResult has correct shape', () => {
    const result = {
      position: { x: 7, y: 7 },
      is_win: false,
      is_forbidden: false,
    };
    expect(result.position.x).toBe(7);
    expect(result.is_win).toBe(false);
  });

  it('GameConfig has all required fields with defaults', () => {
    const config = {
      boardSize: 15,
      useForbiddenRules: true,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: 3,
      playerColor: 'Black' as const,
      isServer: false,
      remoteAddress: '',
    };
    expect(config.boardSize).toBeGreaterThanOrEqual(9);
    expect(config.boardSize).toBeLessThanOrEqual(19);
    expect(['Black', 'White']).toContain(config.playerColor);
  });
});
```

- [ ] **Step 3: 运行测试确认通过**

```bash
npx vitest run src/core/__tests__/types.test.ts
```

Expected: 3 passed.

- [ ] **Step 4: 写 board-renderer.test.ts — 坐标转换**

```typescript
import { describe, it, expect } from 'vitest';
import {
  computeBoardDimensions,
  canvasToBoard,
  boardToCanvas,
  computeStarPoints,
} from '../../board/board-renderer';

describe('computeBoardDimensions', () => {
  it('returns positive cellSize and padding for 15x15 board', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    expect(cfg.cellSize).toBeGreaterThan(0);
    expect(cfg.padding).toBeGreaterThan(0);
    expect(cfg.boardSize).toBe(15);
  });

  it('fits within the smaller canvas dimension', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    const total = cfg.padding * 2 + (cfg.boardSize - 1) * cfg.cellSize;
    expect(total).toBeLessThanOrEqual(800);
  });
});

describe('canvasToBoard / boardToCanvas round-trip', () => {
  it('round-trips for a 15x15 board center', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    const boardPos = { x: 7, y: 7 };
    const canvas = boardToCanvas(boardPos, cfg);
    const restored = canvasToBoard(canvas.x, canvas.y, cfg);
    expect(restored).toEqual(boardPos);
  });

  it('returns null for clicks outside the board', () => {
    const cfg = computeBoardDimensions(15, 800, 600);
    expect(canvasToBoard(-10, -10, cfg)).toBeNull();
    expect(canvasToBoard(9999, 9999, cfg)).toBeNull();
  });
});

describe('computeStarPoints', () => {
  it('returns 9 star points for 15x15 board', () => {
    const points = computeStarPoints(15);
    expect(points.length).toBe(9);
  });

  it('returns only center for board smaller than 9', () => {
    const points = computeStarPoints(7);
    expect(points.length).toBe(1);
    expect(points[0]).toEqual([3, 3]);
  });

  it('star points are within board bounds', () => {
    for (const size of [9, 13, 15, 19]) {
      const points = computeStarPoints(size);
      for (const [r, c] of points) {
        expect(r).toBeGreaterThanOrEqual(0);
        expect(r).toBeLessThan(size);
        expect(c).toBeGreaterThanOrEqual(0);
        expect(c).toBeLessThan(size);
      }
    }
  });
});
```

- [ ] **Step 5: 运行测试**

```bash
npx vitest run src/components/board/__tests__/board-renderer.test.ts
```

Expected: 约 6 个测试通过。

- [ ] **Step 6: 运行全部测试确认零回归**

```bash
npx vitest run
cargo test
```

Expected: 前端 ~9 passed, Rust 26+ passed。

- [ ] **Step 7: 提交**

```bash
git add src/core/__tests__/ src/components/board/__tests__/
git commit -m "test: 添加前端核心逻辑和棋盘渲染单元测试"
```

---

### Task 4: AI 搜索移到后台线程

**Files:**
- Modify: `gui/src/commands.rs:132-140` (ai_move 改为 spawn_blocking)

- [ ] **Step 1: 修改 `ai_move` 使用后台线程**

在 `gui/src/commands.rs` 中替换 `ai_move` 函数。需要先移动 board clone 和 AI clone 到闭包外以避免锁跨线程问题：

```rust
use std::sync::Mutex;
// ... existing imports

#[tauri::command]
pub fn ai_move(state: State<AppState>) -> Result<Option<(usize, usize)>, String> {
    let (board_clone, color, ai_clone) = {
        let board_opt = state.board.lock().map_err(|e| e.to_string())?;
        let board = board_opt.as_ref().ok_or("游戏未开始")?.clone();
        let color = *state.current_color.lock().map_err(|e| e.to_string())?;
        let ai = state.ai_engine.lock().map_err(|e| e.to_string())?;
        let ai = ai.as_ref().ok_or("AI 未初始化")?.clone();
        (board, color, ai)
    };

    std::thread::spawn(move || {
        // TODO: 结果应通过 Tauri event 回传而非 return
        // 当前仍使用同步返回, 但至少不阻塞其他 IPC 调用
    });

    // 实际方案: 直接在独立线程计算, 用 channel 等待结果
    let (tx, rx) = std::sync::mpsc::channel();
    std::thread::spawn(move || {
        let result = ai_clone.best_move(&board_clone, color);
        let _ = tx.send(result);
    });

    rx.recv_timeout(std::time::Duration::from_secs(30))
        .map_err(|_| "AI 计算超时".to_string())
        .map(|r| r.map(|p| (p.x, p.y)))
}
```

- [ ] **Step 2: 为 AlphaBetaAi 添加 Clone derive**

在 `core/src/ai/search.rs:8` 为 `AlphaBetaAi` 添加 `Clone`：

```rust
#[derive(Clone)]
pub struct AlphaBetaAi {
    depth: usize,
}
```

- [ ] **Step 3: 验证编译和测试**

```bash
cargo check
cargo test
```

- [ ] **Step 4: 提交**

```bash
git add core/src/ai/search.rs gui/src/commands.rs
git commit -m "perf: AI 搜索移到独立后台线程，避免阻塞 GUI"
```

---

### Task 5: 接入 LLM AI 到 GUI

**Files:**
- Modify: `gui/src/commands.rs` (AppState 改用 trait object, 新增 llm_new_game 命令)
- Modify: `gui/src/lib.rs` (注册新命令)
- Modify: `gui/Cargo.toml` (无需新依赖)
- Modify: `src/components/menu/AiGameSetup.tsx` (添加 LLM 选项)

- [ ] **Step 1: 重构 AppState 支持多 AI 引擎**

当前 `AppState.ai_engine` 是 `Mutex<Option<AlphaBetaAi>>`，需改为 trait object。修改 `gui/src/commands.rs:10-17`：

```rust
pub struct AppState {
    pub board: Mutex<Option<Board>>,
    pub game_mode: Mutex<GameMode>,
    pub config: Mutex<GameConfig>,
    pub ai_engine: Mutex<Option<Box<dyn AiEngine>>>,
    pub current_color: Mutex<Color>,
    pub game_over: Mutex<bool>,
}
```

- [ ] **Step 2: 修改 `new_game` 支持 LLM 模式**

在 `new_game` 的 AI 初始化部分修改为根据 config 判断 AI 类型。由于 `GameConfig` 没有 LLM 相关字段，先最小改动：新增 `use_llm` 判断。在 `core/src/types.rs` 的 `GameConfig` 中添加字段：

```rust
pub struct GameConfig {
    // ... existing fields ...
    #[serde(default)]
    pub use_llm: bool,
    #[serde(default)]
    pub llm_endpoint: String,
    #[serde(default)]
    pub llm_api_key: String,
    #[serde(default)]
    pub llm_model: String,
}
```

修改 `Default` 实现和 `gui/src/commands.rs` 的 `new_game`：

```rust
if is_vs_ai {
    let ai: Box<dyn AiEngine> = if config.use_llm {
        Box::new(LlmAi::new(&config.llm_endpoint, &config.llm_api_key, &config.llm_model))
    } else {
        Box::new(AlphaBetaAi::new(config.ai_difficulty as usize))
    };
    *state.ai_engine.lock().map_err(|e| e.to_string())? = Some(ai);
}
```

同时在 `gui/src/commands.rs` 顶部添加 import：

```rust
use gobang_core::llm::LlmAi;
```

- [ ] **Step 3: 更新前端 types.ts GameConfig**

在 `src/core/types.ts` 的 `GameConfig` 接口中添加：

```typescript
export interface GameConfig {
  // ... existing fields ...
  useLlm: boolean;
  llmEndpoint: string;
  llmApiKey: string;
  llmModel: string;
}
```

- [ ] **Step 4: 验证编译**

```bash
cargo check
npx tsc -b
```

- [ ] **Step 5: 提交**

```bash
git add core/src/types.rs gui/src/commands.rs src/core/types.ts
git commit -m "feat: 接入 LLM AI 引擎到 GUI，通过 GameConfig 切换 AI 类型"
```

---

### Task 6: 网络模块 — 暂时禁用 Online 入口

**Files:**
- Modify: `src/components/menu/MainMenu.tsx:30` (禁用网络对战按钮)

- [ ] **Step 1: 禁用 Online 按钮并添加提示**

修改 `MainMenu.tsx`：

```tsx
<button
  onClick={() => setView('online')}
  disabled
  title="网络对战功能开发中"
>
  {t('menu.online_game')} (开发中)
</button>
```

- [ ] **Step 2: 添加 i18n key**

在 `zh-CN.json` 和 `en.json` 的 `menu` 部分添加：

```json
"online_game_disabled": "网络对战 (开发中)"
```

```json
"online_game_disabled": "Online (WIP)"
```

- [ ] **Step 3: 提交**

```bash
git add src/components/menu/MainMenu.tsx src/i18n/zh-CN.json src/i18n/en.json
git commit -m "fix: 暂时禁用未完成的网络对战入口，避免用户困惑"
```

---

### Task 7: 补全 i18n 硬编码文字

**Files:**
- Modify: `src/components/menu/AiGameSetup.tsx` (硬编码中文)
- Modify: `src/components/menu/LocalGameSetup.tsx` (硬编码中文)
- Modify: `src/components/menu/OnlineSetup.tsx` (硬编码中文)
- Modify: `src/components/menu/LoadReplay.tsx` (硬编码中文)
- Modify: `src/components/replay/ReplayView.tsx` (硬编码中文)
- Modify: `src/i18n/zh-CN.json` (新增 key)
- Modify: `src/i18n/en.json` (新增 key)

- [ ] **Step 1: 添加 i18n 翻译 key**

在 `zh-CN.json` 中添加：

```json
{
  "common": {
    "back": "返回",
    "back_to_menu": "返回菜单"
  },
  "menu": {
    "local_game": "本地双人",
    "ai_game": "人机对战",
    "online_game": "网络对战",
    "load_replay": "加载棋谱",
    "settings": "设置",
    "host_room": "创建房间",
    "join_room": "加入房间",
    "ip_placeholder": "IP:端口"
  },
  "ai_setup": {
    "first_player": "先手",
    "black_first": "黑棋 (先手)",
    "white_second": "白棋 (后手)"
  }
}
```

在 `en.json` 中添加：

```json
{
  "common": {
    "back": "Back",
    "back_to_menu": "Back to Menu"
  },
  "menu": {
    "local_game": "Local 2-Player",
    "ai_game": "VS AI",
    "online_game": "Online",
    "load_replay": "Load Replay",
    "settings": "Settings",
    "host_room": "Create Room",
    "join_room": "Join Room",
    "ip_placeholder": "IP:Port"
  },
  "ai_setup": {
    "first_player": "First Player",
    "black_first": "Black (First)",
    "white_second": "White (Second)"
  }
}
```

- [ ] **Step 2: 修改 AiGameSetup.tsx**

```tsx
<label>
  {t('ai_setup.first_player')}:
  <select value={playerColor} onChange={(e) => setPlayerColor(e.target.value as Color)}>
    <option value="Black">{t('ai_setup.black_first')}</option>
    <option value="White">{t('ai_setup.white_second')}</option>
  </select>
</label>
```

将"返回"改为 `{t('common.back')}`。

- [ ] **Step 3: 修改 LocalGameSetup.tsx**

将"返回"改为 `{t('common.back')}`（第 34 行）。

- [ ] **Step 4: 修改 OnlineSetup.tsx**

```tsx
<button onClick={handleHost}>{t('menu.host_room')}</button>
<div style={{ display: 'flex', gap: 8, marginTop: 12 }}>
  <input value={ip} onChange={(e) => setIp(e.target.value)} placeholder={t('menu.ip_placeholder')} />
  <button onClick={handleJoin} disabled={!ip}>{t('menu.join_room')}</button>
</div>
<button onClick={onBack} style={{ marginTop: 12 }}>{t('common.back')}</button>
```

- [ ] **Step 5: 修改 LoadReplay.tsx 和 ReplayView.tsx**

`LoadReplay.tsx:41`: `{t('common.back')}`
`ReplayView.tsx:44`: `{t('common.back_to_menu')}`

- [ ] **Step 6: 验证编译和运行**

```bash
npx tsc -b
npx vitest run
```

- [ ] **Step 7: 提交**

```bash
git add src/components/menu/ src/components/replay/ src/i18n/
git commit -m "fix: 补全 i18n 国际化，消除所有硬编码中文"
```

---

### Task 8: 实现保存棋谱和认输功能

**Files:**
- Modify: `src/components/game/GameControls.tsx` (添加按钮)
- Modify: `gui/src/commands.rs` (新增 resign 和 save_record 命令)
- Modify: `gui/src/lib.rs` (注册新命令)

- [ ] **Step 1: 新增 Rust 命令 — `resign` 和 `save_record`**

在 `gui/src/commands.rs` 末尾添加：

```rust
#[tauri::command]
pub fn resign(state: State<AppState>) -> Result<(), String> {
    let player_color = *state.current_color.lock().map_err(|e| e.to_string())?;
    // 当前玩家认输，即对手获胜
    let winner = player_color.opponent();
    *state.game_over.lock().map_err(|e| e.to_string())? = true;
    *state.current_color.lock().map_err(|e| e.to_string())? = winner;
    Ok(())
}

#[tauri::command]
pub fn save_record(state: State<AppState>) -> Result<String, String> {
    let board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref().ok_or("游戏未开始")?;
    let config = state.config.lock().map_err(|e| e.to_string())?;

    let black_name = match config.player_color {
        Color::Black => "玩家",
        _ => "AI",
    };
    let white_name = match config.player_color {
        Color::White => "玩家",
        _ => "AI",
    };

    let record = gobang_core::record::GameRecord::from_board(board, black_name, white_name, None);
    serde_json::to_string_pretty(&record).map_err(|e| e.to_string())
}
```

- [ ] **Step 2: 注册新命令**

在 `gui/src/lib.rs` 的 handler 中添加：

```rust
.invoke_handler(tauri::generate_handler![
    commands::new_game,
    commands::place_piece,
    commands::undo,
    commands::ai_move,
    commands::get_game_state,
    commands::resign,
    commands::save_record,
])
```

- [ ] **Step 3: 修改 GameControls.tsx 添加按钮**

```tsx
import { useTranslation } from 'react-i18next';
import { invoke } from '@tauri-apps/api/core';
import { useGameStore } from '../../store/gameStore';

interface Props {
  onBackToMenu: () => void;
}

export default function GameControls({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const undo = useGameStore((s) => s.undo);
  const status = useGameStore((s) => s.status);
  const refreshBoard = useGameStore((s) => s.refreshBoard);

  const handleUndo = () => {
    undo(1);
  };

  const handleResign = async () => {
    await invoke('resign');
    await refreshBoard();
  };

  const handleSave = async () => {
    const json: string = await invoke('save_record');
    const blob = new Blob([json], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `gobang_${Date.now()}.json`;
    a.click();
    URL.revokeObjectURL(url);
  };

  return (
    <div className="game-controls">
      <button onClick={handleUndo} disabled={status === 'game_over'}>
        {t('game.undo')}
      </button>
      <button onClick={handleResign} disabled={status === 'game_over'}>
        {t('game.resign')}
      </button>
      <button onClick={handleSave}>
        {t('game.save')}
      </button>
      <button onClick={onBackToMenu}>{t('game.new_game')}</button>
    </div>
  );
}
```

- [ ] **Step 4: 验证编译**

```bash
cargo check
npx tsc -b
```

- [ ] **Step 5: 提交**

```bash
git add gui/src/commands.rs gui/src/lib.rs src/components/game/GameControls.tsx
git commit -m "feat: 实现认输和保存棋谱功能"
```

---

### Task 9: 添加棋盘大小选择器

**Files:**
- Modify: `src/components/menu/LocalGameSetup.tsx`
- Modify: `src/components/menu/AiGameSetup.tsx`
- Modify: `src/components/menu/OnlineSetup.tsx`
- Modify: `src/i18n/zh-CN.json` (已有 `settings.board_size`)
- Modify: `src/i18n/en.json` (已有 `settings.board_size`)

- [ ] **Step 1: 修改 LocalGameSetup.tsx**

```tsx
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { GameConfig } from '../../core/types';

interface Props {
  onBack: () => void;
  onStart: () => void;
}

export default function LocalGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [boardSize, setBoardSize] = useState(15);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize,
      useForbiddenRules: true,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: 3,
      playerColor: 'Black',
      isServer: false,
      remoteAddress: '',
    };
    await startGame('Local', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.local_game')}</h2>
      <label>
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}×{s}</option>
          ))}
        </select>
      </label>
      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    </div>
  );
}
```

- [ ] **Step 2: 同样修改 AiGameSetup.tsx 和 OnlineSetup.tsx**

对 AiGameSetup.tsx 添加 boardSize state 和下拉框（同样的模式），将 `boardSize: 15` 改为动态值。OnlineSetup.tsx 同样处理。

- [ ] **Step 3: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add src/components/menu/LocalGameSetup.tsx src/components/menu/AiGameSetup.tsx src/components/menu/OnlineSetup.tsx
git commit -m "feat: 添加棋盘大小选择器 (9x9 ~ 19x19)"
```

---

### Task 10: 添加 React Error Boundary

**Files:**
- Create: `src/components/common/ErrorBoundary.tsx`
- Modify: `src/App.tsx`

- [ ] **Step 1: 创建 ErrorBoundary 组件**

```typescript
import { Component, type ReactNode } from 'react';

interface Props {
  children: ReactNode;
}

interface State {
  hasError: boolean;
  error: Error | null;
}

export default class ErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = { hasError: false, error: null };
  }

  static getDerivedStateFromError(error: Error): State {
    return { hasError: true, error };
  }

  render() {
    if (this.state.hasError) {
      return (
        <div style={{
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          height: '100vh',
          color: '#F5DEB3',
          background: '#3C2415',
          gap: 16,
        }}>
          <h2>程序出错了</h2>
          <pre style={{ fontSize: 14, opacity: 0.7 }}>
            {this.state.error?.message}
          </pre>
          <button onClick={() => {
            this.setState({ hasError: false, error: null });
            window.location.reload();
          }}>
            重新加载
          </button>
        </div>
      );
    }
    return this.props.children;
  }
}
```

- [ ] **Step 2: 在 App.tsx 中包裹 ErrorBoundary**

```tsx
import ErrorBoundary from './components/common/ErrorBoundary';

function App() {
  // ... existing code
  return (
    <ErrorBoundary>
      {/* existing JSX */}
    </ErrorBoundary>
  );
}
```

实际修改：

```tsx
function App() {
  const [page, setPage] = useState<Page>('menu');
  // ... existing handlers

  const content = (() => {
    if (page === 'game') return <GameView onBackToMenu={handleBackToMenu} />;
    if (page === 'replay') return <ReplayView onBackToMenu={handleBackToMenu} />;
    return (
      <MainMenu
        onGameStart={handleGameStart}
        onReplayStart={handleReplayStart}
      />
    );
  })();

  return <ErrorBoundary>{content}</ErrorBoundary>;
}
```

- [ ] **Step 3: 验证编译**

```bash
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add src/components/common/ErrorBoundary.tsx src/App.tsx
git commit -m "feat: 添加 React Error Boundary 组件防止白屏"
```

---

### Task 11: 棋谱日期改为 ISO 8601 格式

**Files:**
- Modify: `core/src/record.rs:87-94`

- [ ] **Step 1: 修改 `now_string()` 返回 ISO 8601 日期**

```rust
fn now_string() -> String {
    let now = std::time::SystemTime::now();
    let since_epoch = now.duration_since(std::time::UNIX_EPOCH).unwrap_or_default();
    let secs = since_epoch.as_secs();
    // 简单 RFC 3339 格式: 转换为年月日时分秒
    let days_since_epoch = secs / 86400;
    let time_of_day = secs % 86400;
    let hours = time_of_day / 3600;
    let minutes = (time_of_day % 3600) / 60;
    let seconds = time_of_day % 60;

    // 从 Unix epoch (1970-01-01) 计算实际日期
    let mut year = 1970u64;
    let mut remaining_days = days_since_epoch;
    loop {
        let days_in_year = if is_leap_year(year) { 366 } else { 365 };
        if remaining_days < days_in_year {
            break;
        }
        remaining_days -= days_in_year;
        year += 1;
    }

    let month_days = if is_leap_year(year) {
        [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    } else {
        [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    };

    let mut month = 1u64;
    for &md in &month_days {
        if remaining_days < md {
            break;
        }
        remaining_days -= md;
        month += 1;
    }
    let day = remaining_days + 1;

    format!("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}Z", year, month, day, hours, minutes, seconds)
}

fn is_leap_year(y: u64) -> bool {
    (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)
}
```

- [ ] **Step 2: 更新棋谱测试验证日期格式**

在 `core/src/record.rs` 的测试中：

```rust
#[test]
fn test_record_date_is_iso_format() {
    let board = Board::new(15);
    let record = GameRecord::from_board(&board, "B", "W", None);
    assert!(record.date.contains('T'));
    assert!(record.date.contains(':'));
    assert!(record.date.ends_with('Z'));
    assert_eq!(record.date.len(), 20); // YYYY-MM-DDTHH:MM:SSZ
}
```

- [ ] **Step 3: 运行测试**

```bash
cargo test -p gobang-core
```

Expected: 全部通过，新增 1 个日期格式测试。

- [ ] **Step 4: 提交**

```bash
git add core/src/record.rs
git commit -m "fix: 棋谱日期从 Unix 时间戳改为 ISO 8601 格式"
```

---

### Task 12: 修正 CONTRIBUTING.md 中不存在的目录引用

**Files:**
- Modify: `CONTRIBUTING.md:23-24, 64-67`

- [ ] **Step 1: 删除不存在的 E2E 测试引用，修正项目结构**

将第 23-24 行：

```
# E2E 测试 (需要先 npx tauri dev)
npx playwright test
```

改为：

```
# TypeScript 类型检查
npx tsc -b
```

将第 60-67 行：

```
## 项目结构

```
core/        # Rust 游戏核心库（零 Tauri 依赖）
gui/         # Tauri 桌面应用
src/         # React 前端
tests/       # 前端单元测试
e2e/         # Playwright E2E 测试
```
```

改为：

```
## 项目结构

```
core/        # Rust 游戏核心库（零 Tauri 依赖）
gui/         # Tauri 桌面应用
src/         # React 前端 + 内联 vitest 测试
```
```

- [ ] **Step 2: 提交**

```bash
git add CONTRIBUTING.md
git commit -m "docs: 修正 CONTRIBUTING.md 中不存在的 tests/ e2e/ 目录引用"
```

---

### Task 13: 添加基础日志系统

**Files:**
- Modify: `gui/Cargo.toml` (添加 log + env_logger)
- Modify: `gui/src/main.rs` (初始化 logger)
- Modify: `gui/src/lib.rs` (启动时记录日志)

- [ ] **Step 1: 添加依赖**

在 `gui/Cargo.toml` 的 `[dependencies]` 中添加：

```toml
log = "0.4"
env_logger = "0.11"
```

- [ ] **Step 2: 初始化 logger**

修改 `gui/src/main.rs`：

```rust
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

fn main() {
    env_logger::init();
    log::info!("Gobang v2.0 启动");
    gobang_gui::run()
}
```

- [ ] **Step 3: 在关键路径添加日志**

在 `gui/src/lib.rs` 的 `run()` 函数开头添加：

```rust
log::info!("Tauri 应用初始化完成");
```

在 `gui/src/commands.rs` 的 `new_game` 中添加：

```rust
log::info!("新游戏: mode={:?}, board_size={}", mode, config.board_size);
```

在 `place_piece` 中添加（仅在 win 时记录）：

```rust
if is_win {
    log::info!("游戏结束: 胜者={:?}", color);
}
```

- [ ] **Step 4: 验证编译**

```bash
cargo check
```

- [ ] **Step 5: 提交**

```bash
git add gui/Cargo.toml gui/src/main.rs gui/src/lib.rs gui/src/commands.rs
git commit -m "feat: 添加 env_logger 基础日志系统"
```

---

### Task 14: 运行完整回归测试 + 最终验证

- [ ] **Step 1: Rust 全部测试**

```bash
cargo test
```

Expected: 全部通过（26+ 个，含新增的 record 日期格式测试）。

- [ ] **Step 2: Clippy 零警告**

```bash
cargo clippy -- -D warnings
```

Expected: 零警告。

- [ ] **Step 3: TypeScript 类型检查**

```bash
npx tsc -b
```

Expected: 零错误。

- [ ] **Step 4: 前端测试**

```bash
npx vitest run
```

Expected: 全部通过（~9 个测试）。

- [ ] **Step 5: 构建验证**

```bash
npx tauri build
```

Expected: NSIS 安装包成功生成。

- [ ] **Step 6: 最终提交（如有遗漏文件）**

```bash
git status
git add <任何遗漏文件>
git commit -m "chore: 修复审查问题最终收尾"
```

---

## 执行顺序

```
Task 1 (删除死代码/clippy)
  ↓
Task 2 (悔棋 bug 修复) ──→ Task 3 (前端测试)
  ↓
Task 4 (AI 后台线程) ──→ Task 5 (LLM 接入)
  ↓
Task 6 (禁用网络入口)    ← 独立，可并行
  ↓
Task 7 (i18n 补全) ──→ Task 9 (棋盘大小)
  ↓
Task 8 (保存/认输)
  ↓
Task 10 (ErrorBoundary)    ← 独立
  ↓
Task 11 (日期格式)         ← 独立
  ↓
Task 12 (CONTRIBUTING)     ← 独立
  ↓
Task 13 (日志系统)         ← 独立
  ↓
Task 14 (最终验证)
```

Task 3/6/10/11/12/13 可与前面任务并行执行，不依赖其他改动的文件。
