use crate::types::{CellState, Color, Move, MoveError, Position, ZobristHash, MAX_BOARD_SIZE};

/// 棋盘主体 — 不可变风格, place/undo 返回新 Board
#[derive(Debug, Clone, PartialEq)]
pub struct Board {
    pub size: usize,
    cells: [[CellState; MAX_BOARD_SIZE]; MAX_BOARD_SIZE],
    history: Vec<Move>,
    current_turn: u32,
    pub zobrist_hash: ZobristHash,
}

impl Board {
    /// 创建空棋盘
    pub fn new(size: usize) -> Self {
        assert!(
            size <= MAX_BOARD_SIZE,
            "棋盘尺寸不能超过 {}",
            MAX_BOARD_SIZE
        );
        Self {
            size,
            cells: [[CellState::Empty; MAX_BOARD_SIZE]; MAX_BOARD_SIZE],
            history: Vec::new(),
            current_turn: 0,
            zobrist_hash: 0,
        }
    }

    /// 获取指定位置的棋子状态
    pub fn get(&self, pos: Position) -> CellState {
        if pos.x >= self.size || pos.y >= self.size {
            return CellState::Empty;
        }
        self.cells[pos.x][pos.y]
    }

    /// 获取当前局面 Zobrist 哈希
    pub fn hash(&self) -> ZobristHash {
        self.zobrist_hash
    }

    /// 落子 — 返回新 Board (不可变)
    pub fn place(&self, pos: Position, color: Color) -> Result<Board, MoveError> {
        if pos.x >= self.size || pos.y >= self.size {
            return Err(MoveError::OutOfBounds);
        }
        if self.cells[pos.x][pos.y] != CellState::Empty {
            return Err(MoveError::Occupied);
        }

        let mut new_board = self.clone();
        new_board.cells[pos.x][pos.y] = CellState::Occupied(color);
        let color_idx = if matches!(color, Color::Black) { 0 } else { 1 };
        let zobrist = crate::types::init_zobrist_table(self.size);
        new_board.zobrist_hash ^= zobrist[pos.x][pos.y][color_idx];
        new_board.history.push(Move {
            position: pos,
            color,
            turn: self.current_turn,
        });
        new_board.current_turn = self.current_turn + 1;
        Ok(new_board)
    }

    /// 胜负判定 — 从 pos 出发四方向扫描
    pub fn check_win(&self, pos: Position) -> bool {
        let cell = self.cells[pos.x][pos.y];
        let color = match cell {
            CellState::Occupied(c) => c,
            _ => return false,
        };

        let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];

        for (dx, dy) in directions {
            let mut count = 1u32;
            // 正方向
            let mut nx = pos.x as isize + dx;
            let mut ny = pos.y as isize + dy;
            while nx >= 0 && ny >= 0 && (nx as usize) < self.size && (ny as usize) < self.size {
                if self.cells[nx as usize][ny as usize] == CellState::Occupied(color) {
                    count += 1;
                    nx += dx;
                    ny += dy;
                } else {
                    break;
                }
            }
            // 反方向
            let mut nx = pos.x as isize - dx;
            let mut ny = pos.y as isize - dy;
            while nx >= 0 && ny >= 0 && (nx as usize) < self.size && (ny as usize) < self.size {
                if self.cells[nx as usize][ny as usize] == CellState::Occupied(color) {
                    count += 1;
                    nx -= dx;
                    ny -= dy;
                } else {
                    break;
                }
            }
            if count >= 5 {
                return true;
            }
        }
        false
    }

    /// 悔棋 — 撤销最近一步
    pub fn undo(&self) -> Result<Board, MoveError> {
        if self.history.is_empty() {
            return Err(MoveError::NoHistory);
        }
        let mut new_board = self.clone();
        let last_move = new_board.history.pop().unwrap();
        new_board.cells[last_move.position.x][last_move.position.y] = CellState::Empty;
        let last_color_idx = if matches!(last_move.color, Color::Black) {
            0
        } else {
            1
        };
        let zobrist = crate::types::init_zobrist_table(self.size);
        new_board.zobrist_hash ^=
            zobrist[last_move.position.x][last_move.position.y][last_color_idx];
        new_board.current_turn = self.current_turn.saturating_sub(1);
        Ok(new_board)
    }

    /// 获取所有候选落子位 (已有棋子周围2格范围)
    pub fn get_candidate_moves(&self) -> Vec<Position> {
        let mut candidates = Vec::new();
        let range = 2isize;

        if self.history.is_empty() {
            // 棋盘为空, 返回天元
            return vec![Position::new(self.size / 2, self.size / 2)];
        }

        for x in 0..self.size {
            for y in 0..self.size {
                if self.cells[x][y] != CellState::Empty {
                    for dx in -range..=range {
                        for dy in -range..=range {
                            let nx = x as isize + dx;
                            let ny = y as isize + dy;
                            if nx >= 0
                                && ny >= 0
                                && (nx as usize) < self.size
                                && (ny as usize) < self.size
                                && self.cells[nx as usize][ny as usize] == CellState::Empty
                            {
                                candidates.push(Position::new(nx as usize, ny as usize));
                            }
                        }
                    }
                }
            }
        }

        candidates.sort();
        candidates.dedup();
        candidates
    }

    /// 获取落子历史 (用于棋谱)
    pub fn history(&self) -> &[Move] {
        &self.history
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::{CellState, Color, MoveError, Position};

    #[test]
    fn test_empty_board_creation() {
        let board = Board::new(15);
        assert_eq!(board.size, 15);
        for x in 0..15 {
            for y in 0..15 {
                assert_eq!(board.get(Position::new(x, y)), CellState::Empty);
            }
        }
    }

    #[test]
    fn test_place_piece() {
        let board = Board::new(15);
        let result = board.place(Position::new(7, 7), Color::Black);
        assert!(result.is_ok());
        let new_board = result.unwrap();
        assert_eq!(
            new_board.get(Position::new(7, 7)),
            CellState::Occupied(Color::Black)
        );
    }

    #[test]
    fn test_place_on_occupied_fails() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let result = board.place(Position::new(7, 7), Color::White);
        assert_eq!(result, Err(MoveError::Occupied));
    }

    #[test]
    fn test_place_out_of_bounds_fails() {
        let board = Board::new(15);
        let result = board.place(Position::new(20, 20), Color::Black);
        assert_eq!(result, Err(MoveError::OutOfBounds));
    }

    #[test]
    fn test_win_horizontal() {
        let board = Board::new(15);
        let mut board = board;
        for y in 3..7 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        assert!(board.check_win(Position::new(7, 7)));
    }

    #[test]
    fn test_win_vertical() {
        let board = Board::new(15);
        let mut board = board;
        for x in 3..7 {
            board = board.place(Position::new(x, 7), Color::White).unwrap();
        }
        let board = board.place(Position::new(7, 7), Color::White).unwrap();
        assert!(board.check_win(Position::new(7, 7)));
    }

    #[test]
    fn test_win_diagonal() {
        let board = Board::new(15);
        let mut board = board;
        for i in 1..5 {
            board = board
                .place(Position::new(3 + i, 3 + i), Color::Black)
                .unwrap();
        }
        let board = board.place(Position::new(8, 8), Color::Black).unwrap();
        assert!(board.check_win(Position::new(8, 8)));
    }

    #[test]
    fn test_no_win_on_four() {
        let board = Board::new(15);
        let mut board = board;
        for y in 3..6 {
            board = board.place(Position::new(7, y), Color::Black).unwrap();
        }
        let board = board.place(Position::new(7, 6), Color::Black).unwrap();
        assert!(!board.check_win(Position::new(7, 6)));
    }

    #[test]
    fn test_undo() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();
        let board = board.undo().unwrap();
        assert_eq!(board.get(Position::new(7, 8)), CellState::Empty);
        assert_eq!(
            board.get(Position::new(7, 7)),
            CellState::Occupied(Color::Black)
        );
    }

    #[test]
    fn test_undo_empty_history_returns_no_history_error() {
        let board = Board::new(15);
        let result = board.undo();
        assert!(result.is_err());
        match result {
            Err(MoveError::NoHistory) => {}
            other => panic!("expected NoHistory, got {:?}", other),
        }
    }

    #[test]
    fn test_immutable_place() {
        let board = Board::new(15);
        let _new = board.place(Position::new(7, 7), Color::Black).unwrap();
        assert_eq!(board.get(Position::new(7, 7)), CellState::Empty);
    }

    #[test]
    fn test_zobrist_hash_changes_on_place() {
        let board = Board::new(15);
        let h1 = board.hash();
        let board2 = board.place(Position::new(7, 7), Color::Black).unwrap();
        assert_ne!(h1, board2.hash());
    }

    #[test]
    fn test_zobrist_hash_restores_on_undo() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let h1 = board.hash();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();
        assert_ne!(h1, board.hash());
        let board = board.undo().unwrap();
        assert_eq!(h1, board.hash());
    }

    #[test]
    fn test_zobrist_hash_symmetry() {
        let board = Board::new(15);
        let b1 = board.place(Position::new(7, 7), Color::Black).unwrap();
        let b2 = board.place(Position::new(7, 8), Color::Black).unwrap();
        assert_ne!(b1.hash(), b2.hash());
    }
}
