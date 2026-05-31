use crate::board::Board;
use crate::types::{CellState, Color, Position};

const FIVE: f64 = 100000.0;
const OPEN_FOUR: f64 = 10000.0;
const RUSH_FOUR: f64 = 5000.0;
const OPEN_THREE: f64 = 1000.0;
const SLEEP_THREE: f64 = 500.0;
const OPEN_TWO: f64 = 100.0;
const SLEEP_TWO: f64 = 50.0;
const OPEN_ONE: f64 = 10.0;

// 组合加分
const COMBO_THREE_THREE: f64 = 5000.0;
const COMBO_THREE_FOUR: f64 = 10000.0;
const COMBO_FOUR_FOUR: f64 = 8000.0;
const COMBO_THREE_TWO: f64 = 500.0;

const POSITION_MAX_BONUS: f64 = 50.0;

/// 评估棋盘对 player 的得分 (player - opponent)
pub fn evaluate_board(board: &Board, player: Color) -> f64 {
    let p_score = evaluate_player(board, player);
    let o_score = evaluate_player(board, player.opponent());
    p_score - o_score
}

fn evaluate_player(board: &Board, color: Color) -> f64 {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    let mut total = 0.0f64;
    let size = board.size;
    let center = (size as f64 - 1.0) / 2.0;

    for x in 0..size {
        for y in 0..size {
            if board.get(Position::new(x, y)) != CellState::Occupied(color) {
                continue;
            }

            let mut patterns: Vec<(u32, u32)> = Vec::with_capacity(4);
            for &(dx, dy) in &directions {
                let (count, open_count, is_start) =
                    scan_pattern(board, Position::new(x, y), color, dx, dy);
                // 始终记录模式信息，用于组合检测（交叉点需要）
                patterns.push((count, open_count));
                // 只在起点处计分，避免重复
                if is_start && count >= 1 {
                    total += score_pattern(count, open_count);
                }
            }

            // 组合棋形：交叉方向检测
            for i in 0..patterns.len() {
                for j in (i + 1)..patterns.len() {
                    let (c1, o1) = patterns[i];
                    let (c2, o2) = patterns[j];
                    if c1 >= 3 && o1 == 2 && c2 >= 3 && o2 == 2 {
                        total += COMBO_THREE_THREE;
                    }
                    if (c1 >= 3 && o1 == 2 && c2 == 4 && o2 == 1)
                        || (c1 == 4 && o1 == 1 && c2 >= 3 && o2 == 2)
                    {
                        total += COMBO_THREE_FOUR;
                    }
                    if c1 == 4 && o1 == 1 && c2 == 4 && o2 == 1 {
                        total += COMBO_FOUR_FOUR;
                    }
                    if (c1 >= 3 && o1 == 2 && c2 == 2 && o2 == 2)
                        || (c1 == 2 && o1 == 2 && c2 >= 3 && o2 == 2)
                    {
                        total += COMBO_THREE_TWO;
                    }
                }
            }

            // 位置权重（高斯分布，中心最高）
            let dx = x as f64 - center;
            let dy = y as f64 - center;
            let dist = (dx * dx + dy * dy).sqrt();
            let max_dist = center;
            total += POSITION_MAX_BONUS * (1.0 - dist / max_dist).max(0.0);
        }
    }
    total
}

/// 扫描从 pos 沿 (dx,dy) 方向的完整棋形。
/// 返回 (总连子数, 开放端数, 是否连续段起点)。
/// 总连子数和开放端数始终正确，供组合检测使用；
/// is_start 用于控制计分，避免重复。
fn scan_pattern(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> (u32, u32, bool) {
    let mut pos_count = 0u32;
    let mut neg_count = 0u32;

    // 正方向
    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Occupied(color)
    {
        pos_count += 1;
        nx += dx;
        ny += dy;
    }
    let end_open = in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty;

    // 反方向
    let mut nx = pos.x as isize - dx;
    let mut ny = pos.y as isize - dy;
    while in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Occupied(color)
    {
        neg_count += 1;
        nx -= dx;
        ny -= dy;
    }
    let start_open = in_bounds(board, nx, ny)
        && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty;

    let total_count = 1 + pos_count + neg_count;
    let open_count = (start_open as u32) + (end_open as u32);
    let is_start = neg_count == 0;

    (total_count, open_count, is_start)
}

fn score_pattern(count: u32, open_count: u32) -> f64 {
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
        assert_eq!(evaluate_board(&board, Color::Black), 0.0);
    }

    #[test]
    fn test_five_in_a_row_high_score() {
        let board = Board::new(15);
        let mut board = board;
        for y in 5..10 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        assert!(evaluate_board(&board, Color::Black) > 10000.0);
    }

    #[test]
    fn test_center_worth_more_than_edge() {
        let board = Board::new(15);
        let b_center = board.place(Position::new(7, 7), Color::Black).unwrap();
        let b_edge = board.place(Position::new(0, 0), Color::Black).unwrap();
        assert!(evaluate_board(&b_center, Color::Black) > evaluate_board(&b_edge, Color::Black));
    }

    #[test]
    fn test_combo_three_three() {
        let board = Board::new(15);
        let mut board = board;
        // 水平活三: (7,5)(7,6)(7,7) — 两端(7,4)(7,8)空
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        board = board.place(Position::new(7, 7), Color::Black).unwrap();
        // 垂直活三: (5,7)(6,7) 与 (7,7) 交叉 — 两端(4,7)(8,7)空
        board = board.place(Position::new(5, 7), Color::Black).unwrap();
        board = board.place(Position::new(6, 7), Color::Black).unwrap();
        let score = evaluate_board(&board, Color::Black);
        assert!(
            score > COMBO_THREE_THREE * 0.5,
            "双活三应大幅加分, got {}",
            score
        );
    }
}
