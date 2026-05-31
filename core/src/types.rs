use serde::{Deserialize, Serialize};
use std::cmp::Ordering;

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
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
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
#[serde(rename_all = "camelCase")]
pub struct GameConfig {
    pub board_size: usize,
    pub use_forbidden_rules: bool,
    pub use_timer: bool,
    pub time_limit_secs: u32,
    pub ai_difficulty: u32,
    pub player_color: Color,
    pub is_server: bool,
    #[serde(default)]
    pub remote_address: String,
    #[serde(default)]
    pub host_port: u16,
    #[serde(default)]
    pub use_llm: bool,
    #[serde(default)]
    pub llm_endpoint: String,
    #[serde(default)]
    pub llm_api_key: String,
    #[serde(default)]
    pub llm_model: String,
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
            remote_address: String::new(),
            host_port: 0,
            use_llm: false,
            llm_endpoint: String::new(),
            llm_api_key: String::new(),
            llm_model: String::new(),
        }
    }
}

/// Zobrist 哈希值
pub type ZobristHash = u64;

/// 获取全局 Zobrist 随机表（只初始化一次，使用 MAX_BOARD_SIZE 确保所有棋盘尺寸可用）
pub fn init_zobrist_table(_board_size: usize) -> &'static Vec<Vec<[ZobristHash; 2]>> {
    use std::collections::hash_map::RandomState;
    use std::hash::BuildHasher;
    use std::sync::OnceLock;
    static TABLE: OnceLock<Vec<Vec<[ZobristHash; 2]>>> = OnceLock::new();
    TABLE.get_or_init(|| {
        let size = MAX_BOARD_SIZE;
        let rng = RandomState::new();
        let mut table = Vec::with_capacity(size);
        for x in 0..size {
            let mut row = Vec::with_capacity(size);
            for _y in 0..size {
                row.push([rng.hash_one((x, _y, 0u8)), rng.hash_one((x, _y, 1u8))]);
            }
            table.push(row);
        }
        table
    })
}
