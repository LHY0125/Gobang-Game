use crate::board::Board;
use crate::scan;
use crate::types::{CellState, Color, Position};

pub fn is_forbidden(board: &Board, pos: Position, color: Color) -> bool {
    if color == Color::White {
        return false;
    }
    is_overline(board, pos, color)
        || is_double_three(board, pos, color)
        || is_double_four(board, pos, color)
}

fn is_overline(board: &Board, pos: Position, color: Color) -> bool {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let info = scan::scan_direction(board, pos, color, dx, dy);
        if info.count >= 6 {
            return true;
        }
    }
    false
}

fn is_double_three(board: &Board, pos: Position, color: Color) -> bool {
    count_open_threes(board, pos, color) >= 2
}

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

fn is_open_three_in_direction(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> bool {
    let info = scan::scan_direction(board, pos, color, dx, dy);
    if info.count != 3 || !info.start_open || !info.end_open {
        return false;
    }
    // 严格活三: 至少一端放入第4子后仍为活四（即该端再往外一格也是空）
    can_extend_to_open_four(board, pos, color, dx, dy, info)
}

/// 检查活三至少一端能延伸成活四（该端外侧第2格为空或可落子）
fn can_extend_to_open_four(
    board: &Board,
    pos: Position,
    _color: Color,
    dx: isize,
    dy: isize,
    info: scan::LineInfo,
) -> bool {
    // 正向端外侧: 从 pos 出发沿 +dx,+dy 方向走 count 步后，再往外一格
    let ex = pos.x as isize + dx * info.count as isize;
    let ey = pos.y as isize + dy * info.count as isize;
    if scan::cell_at(board, ex, ey) == Some(CellState::Empty) {
        return true;
    }
    // 反向端外侧: 从 pos 出发沿 -dx,-dy 方向走 count 步后，再往外一格
    let sx = pos.x as isize - dx * info.count as isize;
    let sy = pos.y as isize - dy * info.count as isize;
    scan::cell_at(board, sx, sy) == Some(CellState::Empty)
}

fn is_four_in_direction(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> bool {
    let info = scan::scan_direction(board, pos, color, dx, dy);
    info.count == 4
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::Position;

    #[test]
    fn test_double_three_forbidden() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 5), Color::Black).unwrap();
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        let board = board.place(Position::new(5, 9), Color::Black).unwrap();
        let board = board.place(Position::new(6, 8), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 7), Color::Black));
    }

    #[test]
    fn test_double_four_forbidden() {
        let board = Board::new(15);
        let mut board = board;
        board = board.place(Position::new(7, 4), Color::Black).unwrap();
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        board = board.place(Position::new(4, 4), Color::Black).unwrap();
        board = board.place(Position::new(5, 5), Color::Black).unwrap();
        board = board.place(Position::new(6, 6), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 7), Color::Black));
    }

    #[test]
    fn test_overline_forbidden() {
        let board = Board::new(15);
        let mut board = board;
        for y in 1..6 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        assert!(is_forbidden(&board, Position::new(7, 6), Color::Black));
    }

    #[test]
    fn test_white_not_forbidden() {
        let board = Board::new(15);
        let mut board = board;
        for y in 1..6 {
            board = board.place(Position::new(7, y), Color::White).unwrap();
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

    #[test]
    fn test_fake_open_three_not_forbidden() {
        // 3子一端被堵，不是真正活三
        let board = Board::new(15);
        let board = board.place(Position::new(7, 5), Color::White).unwrap(); // 堵住一端
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::Black).unwrap();
        // 在(7,4)落子只有水平方向3连但一端被白棋堵，非活三
        // 但如果在另一端(7,9)有另一方向的活三则可能禁手
        // 仅测试单方向不是活三
        assert!(!is_forbidden(&board, Position::new(7, 4), Color::Black));
    }

    #[test]
    fn test_edge_open_three_detection() {
        // 边角的活三：一端靠边界的活三不算真正的活三
        let board = Board::new(15);
        let board = board.place(Position::new(0, 1), Color::Black).unwrap();
        let board = board.place(Position::new(0, 2), Color::Black).unwrap();
        let board = board.place(Position::new(0, 3), Color::Black).unwrap();
        // 在(0,0)落子，水平方向形成 4 连（0,0..0,3），不是活三
        // (0,4)是空，所以是冲四，不是活三
        assert!(!is_forbidden(&board, Position::new(0, 0), Color::Black));
    }
}
