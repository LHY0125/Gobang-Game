use serde::{Deserialize, Serialize};
use crate::board::Board;
use crate::types::{Color, Position};

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
            date: now_string(),
            board_size,
            black_player: black.to_string(),
            white_player: white.to_string(),
            winner: None,
            moves: Vec::new(),
        }
    }

    /// 从棋盘状态构建棋谱
    pub fn from_board(board: &Board, black: &str, white: &str, winner: Option<Color>) -> Self {
        let winner_str = winner.map(|c| match c {
            Color::Black => black.to_string(),
            Color::White => white.to_string(),
        });
        let moves = board.history().iter().map(|m| RecordMove {
            x: m.position.x,
            y: m.position.y,
            color: match m.color {
                Color::Black => "Black".into(),
                Color::White => "White".into(),
            },
            turn: m.turn,
        }).collect();

        Self {
            version: "2.0".to_string(),
            date: now_string(),
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
            board = board
                .place(Position::new(m.x, m.y), color)
                .map_err(|e| e.to_string())?;
        }
        Ok(board)
    }
}

fn now_string() -> String {
    use std::time::SystemTime;
    if let Ok(dur) = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
        format!("{}", dur.as_secs())
    } else {
        "unknown".to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{CellState, Color, Position};

    #[test]
    fn test_save_and_load_record() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();

        let record =
            GameRecord::from_board(&board, "Human", "AI-Lv3", Some(Color::Black));
        let json = serde_json::to_string_pretty(&record).unwrap();

        let loaded: GameRecord = serde_json::from_str(&json).unwrap();
        assert_eq!(loaded.moves.len(), 2);
        assert_eq!(loaded.moves[0].x, 7);
        assert_eq!(loaded.moves[0].y, 7);
    }

    #[test]
    fn test_replay_board() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();

        let record = GameRecord::from_board(&board, "Human", "AI", None);
        let replayed = record.to_replay_board().unwrap();

        assert_eq!(
            replayed.get(Position::new(7, 7)),
            CellState::Occupied(Color::Black)
        );
        assert_eq!(
            replayed.get(Position::new(7, 8)),
            CellState::Occupied(Color::White)
        );
    }
}
