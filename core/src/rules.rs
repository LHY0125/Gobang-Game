use crate::board::Board;
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
        let mut count = 1u32;
        let mut nx = pos.x as isize + dx;
        let mut ny = pos.y as isize + dy;
        while let Some(cell) = get_cell(board, nx, ny) {
            if cell == CellState::Occupied(color) {
                count += 1;
            } else {
                break;
            }
            nx += dx;
            ny += dy;
        }
        let mut nx = pos.x as isize - dx;
        let mut ny = pos.y as isize - dy;
        while let Some(cell) = get_cell(board, nx, ny) {
            if cell == CellState::Occupied(color) {
                count += 1;
            } else {
                break;
            }
            nx -= dx;
            ny -= dy;
        }
        if count >= 6 {
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
    let (cnt, start_open, end_open) = scan_direction(board, pos, color, dx, dy);
    cnt == 3 && start_open && end_open
}

fn is_four_in_direction(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> bool {
    let (cnt, _start_open, _end_open) = scan_direction(board, pos, color, dx, dy);
    cnt == 4
}

fn scan_direction(
    board: &Board,
    pos: Position,
    color: Color,
    dx: isize,
    dy: isize,
) -> (u32, bool, bool) {
    let mut count = 1u32;

    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while let Some(cell) = get_cell(board, nx, ny) {
        if cell == CellState::Occupied(color) {
            count += 1;
        } else {
            break;
        }
        nx += dx;
        ny += dy;
    }
    let end_open = get_cell(board, nx, ny) == Some(CellState::Empty);

    let mut nx = pos.x as isize - dx;
    let mut ny = pos.y as isize - dy;
    while let Some(cell) = get_cell(board, nx, ny) {
        if cell == CellState::Occupied(color) {
            count += 1;
        } else {
            break;
        }
        nx -= dx;
        ny -= dy;
    }
    let start_open = get_cell(board, nx, ny) == Some(CellState::Empty);

    (count, start_open, end_open)
}

fn get_cell(board: &Board, x: isize, y: isize) -> Option<CellState> {
    if x < 0 || y < 0 || x as usize >= board.size || y as usize >= board.size {
        return None;
    }
    Some(board.get(Position::new(x as usize, y as usize)))
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
        // 水平方向: (7,4)(7,5)(7,6) 已落黑子, 在(7,7)落子形成活四 (4子)
        board = board.place(Position::new(7, 4), Color::Black).unwrap();
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        // 对角线(1,1)方向: (4,4)(5,5)(6,6) 已落黑子, 在(7,7)形成另一个活四
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
}
