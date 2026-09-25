use gobang_core::ai::search::AlphaBetaAi;
use gobang_core::ai::AiEngine;
use gobang_core::board::Board;
use gobang_core::llm::LlmAi;
use gobang_core::network::{NetworkCmd, NetworkEvent, NetworkLoop};
use gobang_core::rules;
use gobang_core::types::*;
use std::sync::mpsc;
use std::sync::{Arc, Mutex, RwLock};
use tauri::{Emitter, State};

pub(crate) struct GameSession {
    board: Option<Board>,
    mode: GameMode,
    config: GameConfig,
    current_color: Color,
    game_over: bool,
    winner: Option<Color>,
}

impl Default for GameSession {
    fn default() -> Self {
        Self {
            board: None,
            mode: GameMode::Local,
            config: GameConfig::default(),
            current_color: Color::Black,
            game_over: true,
            winner: None,
        }
    }
}

/// 应用全局状态
pub struct AppState {
    pub session: RwLock<GameSession>,
    pub ai_engine: Mutex<Option<Arc<dyn AiEngine + Send + Sync>>>,
    pub ai_busy: Mutex<bool>,
    pub network_tx: Mutex<Option<mpsc::Sender<NetworkCmd>>>,
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            session: RwLock::new(GameSession::default()),
            ai_engine: Mutex::new(None),
            ai_busy: Mutex::new(false),
            network_tx: Mutex::new(None),
        }
    }
}

// ──────────────────────── 命令实现 ────────────────────────

#[tauri::command]
pub fn new_game(mode: GameMode, config: GameConfig, state: State<AppState>) -> Result<(), String> {
    // 清理旧的网络连接
    if let Ok(mut tx) = state.network_tx.lock() {
        if let Some(tx) = tx.take() {
            let _ = tx.send(NetworkCmd::Shutdown);
        }
    }

    let is_vs_ai = mode == GameMode::VsAi;
    let board = Board::new(config.rules.board_size);
    log::info!(
        "新游戏: mode={:?}, board_size={}",
        mode,
        config.rules.board_size
    );

    let mut session = state.session.write().map_err(|e| e.to_string())?;
    session.board = Some(board);
    session.mode = mode;
    session.config = config.clone();
    session.current_color = config.ai.player_color;
    session.game_over = false;
    session.winner = None;
    drop(session);

    // 初始化 AI
    if is_vs_ai {
        let ai: Arc<dyn AiEngine + Send + Sync> = if config.ai.use_llm {
            Arc::new(LlmAi::new(
                &config.ai.llm_endpoint,
                &config.ai.llm_api_key,
                &config.ai.llm_model,
            ))
        } else {
            Arc::new(AlphaBetaAi::new(config.ai.ai_difficulty as usize))
        };
        *state.ai_engine.lock().map_err(|e| e.to_string())? = Some(ai);
    }

    Ok(())
}

#[tauri::command]
pub fn place_piece(x: usize, y: usize, state: State<AppState>) -> Result<MoveResult, String> {
    let mut session = state.session.write().map_err(|e| e.to_string())?;
    if session.game_over {
        return Err("游戏已结束".into());
    }

    let color = session.current_color;
    let pos = Position::new(x, y);
    let board = session.board.as_ref().ok_or("游戏未开始")?;

    // 禁手检查
    if session.config.rules.use_forbidden_rules && rules::is_forbidden(board, pos, color) {
        return Err("禁手位置，不能落子".into());
    }

    let new_board = board.place(pos, color).map_err(|e| e.to_string())?;
    let is_win = new_board.check_win(pos);

    session.board = Some(new_board);
    session.current_color = color.opponent();
    session.game_over = is_win;
    if is_win {
        session.winner = Some(color);
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
    let mut session = state.session.write().map_err(|e| e.to_string())?;
    let mut board = session.board.clone().ok_or("游戏未开始")?;

    let max_undo = board.history().len() as u32;
    let actual_steps = (steps * 2).min(max_undo);

    for _ in 0..actual_steps {
        board = board.undo().map_err(|e| e.to_string())?;
    }

    session.current_color = match board.history().last() {
        Some(last_move) => last_move.color.opponent(),
        None => session.config.ai.player_color,
    };
    session.game_over = false;
    session.winner = None;
    session.board = Some(board);
    Ok(())
}

#[tauri::command]
pub fn ai_move(state: State<AppState>) -> Result<Option<(usize, usize)>, String> {
    // 去重保护
    {
        let mut busy = state.ai_busy.lock().map_err(|e| e.to_string())?;
        if *busy {
            return Err("AI 正在计算中".into());
        }
        *busy = true;
    }

    let (board_clone, color, ai_arc) = {
        let session = state.session.read().map_err(|e| e.to_string())?;
        let board = session.board.as_ref().ok_or("游戏未开始")?.clone();
        let ai_guard = state.ai_engine.lock().map_err(|e| e.to_string())?;
        let ai_arc = ai_guard.as_ref().ok_or("AI 未初始化")?.clone();
        (board, session.current_color, ai_arc)
    };

    let (tx, rx) = std::sync::mpsc::channel();
    std::thread::spawn(move || {
        let result = ai_arc.best_move(&board_clone, color);
        let _ = tx.send(result);
    });

    let result = rx
        .recv_timeout(std::time::Duration::from_secs(30))
        .map_err(|_| "AI 计算超时".to_string())
        .map(|r| r.map(|p| (p.x, p.y)));

    // 解除去重保护
    *state.ai_busy.lock().map_err(|e| e.to_string())? = false;

    result
}

#[tauri::command]
pub async fn ai_move_llm(
    state: State<'_, AppState>,
    app: tauri::AppHandle,
) -> Result<Option<(usize, usize)>, String> {
    // 去重保护
    {
        let mut busy = state.ai_busy.lock().map_err(|e| e.to_string())?;
        if *busy {
            return Err("AI 正在计算中".into());
        }
        *busy = true;
    }

    let (board_clone, color, endpoint, api_key, model) = {
        let session = state.session.read().map_err(|e| e.to_string())?;
        let board = session.board.as_ref().ok_or("游戏未开始")?.clone();
        let config = &session.config;
        (
            board,
            session.current_color,
            config.ai.llm_endpoint.clone(),
            config.ai.llm_api_key.clone(),
            config.ai.llm_model.clone(),
        )
    };

    let llm = LlmAi::new(&endpoint, &api_key, &model);

    let full_content = llm
        .stream_move(&board_clone, color, &|token: &str| {
            let _ = app.emit("llm-stream", token);
        })
        .await?;

    let pos = LlmAi::parse_response(&full_content);
    if let Some(p) = pos {
        let _ = app.emit("llm-move", serde_json::json!({"x": p.x, "y": p.y}));
    }

    *state.ai_busy.lock().map_err(|e| e.to_string())? = false;
    Ok(pos.map(|p| (p.x, p.y)))
}

#[tauri::command]
pub fn get_game_state(state: State<AppState>) -> Result<serde_json::Value, String> {
    let session = state.session.read().map_err(|e| e.to_string())?;

    let cells: Vec<Vec<i32>> = session
        .board
        .as_ref()
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
        "current_color": match session.current_color { Color::Black => "Black", Color::White => "White" },
        "game_over": session.game_over,
        "winner": session.winner.map(|c| match c { Color::Black => "Black", Color::White => "White" }),
    }))
}

#[tauri::command]
pub fn resign(state: State<AppState>) -> Result<(), String> {
    let mut session = state.session.write().map_err(|e| e.to_string())?;
    let winner = session.current_color.opponent();
    session.game_over = true;
    session.winner = Some(winner);
    Ok(())
}

#[tauri::command]
pub fn save_record(state: State<AppState>) -> Result<String, String> {
    let session = state.session.read().map_err(|e| e.to_string())?;
    let board = session.board.as_ref().ok_or("游戏未开始")?;
    let record = gobang_core::record::GameRecord::from_board(board, "玩家", "对手", None);
    serde_json::to_string_pretty(&record).map_err(|e| e.to_string())
}

// ──────────────────────── 网络 ────────────────────────

fn spawn_event_forwarder(event_rx: mpsc::Receiver<NetworkEvent>, app: tauri::AppHandle) {
    std::thread::spawn(move || {
        for event in event_rx {
            match event {
                NetworkEvent::RemoteMove { x, y } => {
                    let _ = app.emit("remote-move", serde_json::json!({ "x": x, "y": y }));
                }
                NetworkEvent::RemoteUndo { steps } => {
                    let _ = app.emit("remote-undo", steps);
                }
                NetworkEvent::RemoteResign => {
                    let _ = app.emit("remote-resign", ());
                }
                NetworkEvent::Connected | NetworkEvent::ClientConnected => {
                    let _ = app.emit("connection-status", "connected");
                }
                NetworkEvent::ClientDisconnected => {
                    let _ = app.emit("connection-status", "disconnected");
                }
                NetworkEvent::Error(msg) => {
                    let _ = app.emit("network-error", msg);
                }
                NetworkEvent::Listening(port) => {
                    let _ = app.emit("listening-port", port);
                }
            }
        }
    });
}

#[tauri::command]
pub fn host_game(port: u16, state: State<AppState>, app: tauri::AppHandle) -> Result<u16, String> {
    let (cmd_tx, cmd_rx) = mpsc::channel();
    let (event_tx, event_rx) = mpsc::channel();

    let mut network = NetworkLoop::new_server(cmd_rx, event_tx);
    let sock = std::net::UdpSocket::bind(format!("0.0.0.0:{}", port))
        .map_err(|e| format!("绑定端口失败: {}", e))?;
    let actual_port = sock.local_addr().map_err(|e| e.to_string())?.port();
    drop(sock);

    *state.network_tx.lock().map_err(|e| e.to_string())? = Some(cmd_tx);

    let protocol_id: u64 = 7777;
    std::thread::spawn(move || {
        let _ = network.run("", protocol_id);
    });

    spawn_event_forwarder(event_rx, app.clone());
    Ok(actual_port)
}

#[tauri::command]
pub fn join_game(
    address: String,
    state: State<AppState>,
    app: tauri::AppHandle,
) -> Result<(), String> {
    let (cmd_tx, cmd_rx) = mpsc::channel();
    let (event_tx, event_rx) = mpsc::channel();

    let mut network = NetworkLoop::new_client(cmd_rx, event_tx);

    *state.network_tx.lock().map_err(|e| e.to_string())? = Some(cmd_tx);

    let protocol_id: u64 = 7777;
    let addr = address.clone();
    std::thread::spawn(move || {
        let _ = network.run(&addr, protocol_id);
    });

    spawn_event_forwarder(event_rx, app.clone());
    Ok(())
}

#[tauri::command]
pub fn send_move(x: usize, y: usize, turn: u32, state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendMove { x, y, turn })
        .map_err(|e| e.to_string())
}

#[tauri::command]
pub fn send_undo(steps: u32, state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendUndo { steps })
        .map_err(|e| e.to_string())
}

#[tauri::command]
pub fn send_resign(state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendResign).map_err(|e| e.to_string())
}
