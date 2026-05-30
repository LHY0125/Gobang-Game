use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 棋形分数
const FIVE: f64 = 100000.0;
const OPEN_FOUR: f64 = 10000.0;
const RUSH_FOUR: f64 = 5000.0;
const OPEN_THREE: f64 = 1000.0;
const SLEEP_THREE: f64 = 500.0;
const OPEN_TWO: f64 = 100.0;
const SLEEP_TWO: f64 = 50.0;
const OPEN_ONE: f64 = 10.0;

/// 评估整个棋盘对 player 的得分 (player得分 - 对手得分)
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

/// 从 pos 向 (dx,dy) 方向扫描, 只计数起点
fn scan_pattern(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> (u32, bool, bool) {
    let mut count = 1u32;

    // 正方向
    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Occupied(color)
    {
        count += 1;
        nx += dx;
        ny += dy;
    }
    let end_open = in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty;

    // 反方向 (检查是否是起点)
    let sx = pos.x as isize - dx;
    let sy = pos.y as isize - dy;
    let start_open = in_bounds(board, sx, sy)
        && board.get(Position::new(sx as usize, sy as usize)) == CellState::Empty;

    // 如果不是连续段的起点, 不计分 (避免重复)
    if in_bounds(board, sx, sy)
        && board.get(Position::new(sx as usize, sy as usize)) == CellState::Occupied(color)
    {
        return (0, false, false);
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_evaluate_empty_board() {
        let board = Board::new(15);
        let score = evaluate_board(&board, Color::Black);
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
        assert!(score > 10000.0);
    }
}
