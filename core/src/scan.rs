use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 方向扫描结果
#[derive(Debug, Clone, Copy)]
pub struct LineInfo {
    /// 连续同色棋子数（含 pos 本身）
    pub count: u32,
    /// 起始端是否开放（空位）
    pub start_open: bool,
    /// 结束端是否开放（空位）
    pub end_open: bool,
}

/// 从 pos 沿 (dx, dy) 方向扫描连续同色棋子。
/// 返回完整线段信息（始终包含 pos 自身）。
pub fn scan_direction(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> LineInfo {
    let mut count = 1u32;

    // 正方向
    let (pos_count, end_open) = count_in_direction(board, pos, color, dx, dy);
    count += pos_count;

    // 反方向
    let (neg_count, start_open) = count_in_direction(board, pos, color, -dx, -dy);
    count += neg_count;

    LineInfo {
        count,
        start_open,
        end_open,
    }
}

/// 从 pos+1 开始沿方向计数连续同色棋子。
/// 返回 (连续数, 末端是否为空位)。
fn count_in_direction(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> (u32, bool) {
    let mut count = 0u32;
    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;

    loop {
        match cell_at(board, nx, ny) {
            Some(CellState::Occupied(c)) if c == color => {
                count += 1;
                nx += dx;
                ny += dy;
            }
            Some(CellState::Occupied(_)) => return (count, false),
            Some(CellState::Empty) => return (count, true),
            None => return (count, false),
        }
    }
}

/// 安全获取棋盘格状态（越界返回 None）
pub fn cell_at(board: &Board, x: isize, y: isize) -> Option<CellState> {
    if x < 0 || y < 0 || (x as usize) >= board.size || (y as usize) >= board.size {
        return None;
    }
    Some(board.get(Position::new(x as usize, y as usize)))
}
