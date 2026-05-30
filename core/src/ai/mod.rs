use crate::board::Board;
use crate::types::{Color, Position};

/// AI 引擎统一接口
pub trait AiEngine: Send + Sync {
    /// 返回 AI 的最佳落子位置, 无位置返回 None
    fn best_move(&self, board: &Board, color: Color) -> Option<Position>;
}

pub mod evaluate;
pub mod search;
