use gobang_core::ai::search::AlphaBetaAi;
use gobang_core::ai::AiEngine;
use gobang_core::board::Board;
use gobang_core::rules;
use gobang_core::types::*;
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
pub fn new_game(mode: GameMode, config: GameConfig, state: State<AppState>) -> Result<(), String> {
    let is_vs_ai = mode == GameMode::VsAi;
    let board = Board::new(config.board_size);
    log::info!("新游戏: mode={:?}, board_size={}", mode, config.board_size);
    *state.board.lock().map_err(|e| e.to_string())? = Some(board);
    *state.game_mode.lock().map_err(|e| e.to_string())? = mode;
    *state.config.lock().map_err(|e| e.to_string())? = config.clone();
    *state.current_color.lock().map_err(|e| e.to_string())? = config.player_color;
    *state.game_over.lock().map_err(|e| e.to_string())? = false;

    // 初始化 AI (如果是人机模式)
    if is_vs_ai {
        let ai = AlphaBetaAi::new(config.ai_difficulty as usize);
        *state.ai_engine.lock().map_err(|e| e.to_string())? = Some(ai);
    }

    Ok(())
}

#[tauri::command]
pub fn place_piece(x: usize, y: usize, state: State<AppState>) -> Result<MoveResult, String> {
    // 检查游戏是否结束
    {
        let game_over = state.game_over.lock().map_err(|e| e.to_string())?;
        if *game_over {
            return Err("游戏已结束".into());
        }
    }

    let color = *state.current_color.lock().map_err(|e| e.to_string())?;
    let pos = Position::new(x, y);

    // 在作用域内验证并落子，确保 board 锁在写入前释放
    let (new_board, is_win) = {
        let board_opt = state.board.lock().map_err(|e| e.to_string())?;
        let board = board_opt.as_ref().ok_or("游戏未开始")?;
        let config = state.config.lock().map_err(|e| e.to_string())?;

        // 禁手检查
        if config.use_forbidden_rules && rules::is_forbidden(board, pos, color) {
            return Err("禁手位置，不能落子".into());
        }

        let new_board = board.place(pos, color).map_err(|e| e.to_string())?;
        let is_win = new_board.check_win(pos);
        (new_board, is_win)
    };

    // 更新游戏状态（前面作用域内的锁已全部释放）
    *state.board.lock().map_err(|e| e.to_string())? = Some(new_board);
    *state.current_color.lock().map_err(|e| e.to_string())? = color.opponent();
    *state.game_over.lock().map_err(|e| e.to_string())? = is_win;

    if is_win {
        log::info!("游戏结束: 胜者={:?}", color);
    }

    Ok(MoveResult {
        position: pos,
        is_win,
        is_forbidden: false,
    })
}

#[tauri::command]
pub fn undo(steps: u32, state: State<AppState>) -> Result<(), String> {
    let mut board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let mut board = board_opt.clone().ok_or("游戏未开始")?;

    let max_undo = board.history().len() as u32;
    let actual_steps = (steps * 2).min(max_undo);

    for _ in 0..actual_steps {
        board = board.undo().map_err(|e| e.to_string())?;
    }

    // 根据剩余步数修正当前颜色 (偶数 = 黑, 奇数 = 白)
    let corrected_color = match board.history().last() {
        Some(last_move) => last_move.color.opponent(),
        None => state.config.lock().map_err(|e| e.to_string())?.player_color,
    };
    *state.current_color.lock().map_err(|e| e.to_string())? = corrected_color;
    *state.game_over.lock().map_err(|e| e.to_string())? = false;

    *board_opt = Some(board);
    Ok(())
}


#[tauri::command]
pub fn ai_move(state: State<AppState>) -> Result<Option<(usize, usize)>, String> {
    let (board_clone, color, ai_clone) = {
        let board_opt = state.board.lock().map_err(|e| e.to_string())?;
        let board = board_opt.as_ref().ok_or("游戏未开始")?.clone();
        let color = *state.current_color.lock().map_err(|e| e.to_string())?;
        let ai_guard = state.ai_engine.lock().map_err(|e| e.to_string())?;
        let ai = ai_guard.as_ref().ok_or("AI 未初始化")?.clone();
        (board, color, ai)
    };

    let (tx, rx) = std::sync::mpsc::channel();
    std::thread::spawn(move || {
        let result = ai_clone.best_move(&board_clone, color);
        let _ = tx.send(result);
    });

    rx.recv_timeout(std::time::Duration::from_secs(30))
        .map_err(|_| "AI 计算超时".to_string())
        .map(|r| r.map(|p| (p.x, p.y)))
}

#[tauri::command]
pub fn get_game_state(state: State<AppState>) -> Result<serde_json::Value, String> {
    let board_opt = state.board.lock().map_err(|e| e.to_string())?;
    let color = *state.current_color.lock().map_err(|e| e.to_string())?;
    let game_over = *state.game_over.lock().map_err(|e| e.to_string())?;
    let board = board_opt.as_ref();

    let cells: Vec<Vec<i32>> = board
        .map(|b| {
            (0..b.size)
                .map(|x| {
                    (0..b.size)
                        .map(move |y| match b.get(Position::new(x, y)) {
                            CellState::Empty => 0,
                            CellState::Occupied(Color::Black) => 1,
                            CellState::Occupied(Color::White) => 2,
                        })
                        .collect()
                })
                .collect()
        })
        .unwrap_or_default();

    Ok(serde_json::json!({
        "board": cells,
        "current_color": match color { Color::Black => "Black", Color::White => "White" },
        "game_over": game_over,
    }))
}
