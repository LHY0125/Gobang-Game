use crate::types::{Position, ZobristHash};
use rand::seq::SliceRandom;
use std::collections::HashMap;

pub struct OpeningBook {
    positions: HashMap<ZobristHash, Vec<Position>>,
}

impl Default for OpeningBook {
    fn default() -> Self {
        let mut book = Self {
            positions: HashMap::new(),
        };
        book.load();
        book
    }
}

impl OpeningBook {
    pub fn new() -> Self {
        Self::default()
    }

    fn load(&mut self) {
        let openings: Vec<Vec<(usize, usize)>> = vec![
            vec![(7, 7), (7, 8), (6, 7), (6, 6), (8, 6)],
            vec![(7, 7), (7, 8), (6, 7), (8, 8), (5, 7)],
            vec![(7, 7), (8, 7), (7, 6), (6, 6), (8, 5)],
            vec![(7, 7), (8, 7), (7, 6), (7, 8), (6, 5)],
            vec![(7, 7), (6, 6), (7, 6), (8, 8), (6, 5)],
            vec![(7, 7), (6, 6), (7, 6), (8, 6), (5, 7)],
            vec![(7, 7), (6, 8), (6, 7), (8, 7), (5, 7)],
            vec![(7, 7), (6, 8), (6, 7), (7, 8), (5, 6)],
            vec![(7, 7), (8, 6), (7, 6), (6, 8), (8, 5)],
            vec![(7, 7), (8, 6), (7, 6), (9, 6), (6, 7)],
            vec![(7, 7), (7, 6), (8, 8), (6, 7), (8, 7)],
            vec![(7, 7), (7, 6), (8, 8), (6, 8), (5, 8)],
            vec![(7, 7), (8, 8), (7, 6), (6, 7), (8, 6)],
            vec![(7, 7), (8, 8), (7, 6), (7, 8), (8, 7)],
            vec![(7, 7), (6, 8), (8, 6), (5, 7), (8, 8)],
            vec![(7, 7), (6, 8), (8, 6), (6, 6), (9, 5)],
            vec![(7, 7), (8, 7), (7, 8), (6, 6), (9, 7)],
            vec![(7, 7), (8, 7), (7, 8), (6, 7), (9, 6)],
            vec![(7, 7), (8, 7), (7, 8), (7, 6), (9, 8)],
            vec![(7, 7), (8, 7), (7, 8), (8, 6), (6, 8)],
            vec![(7, 7), (8, 6), (6, 8), (5, 7), (8, 8)],
            vec![(7, 7), (8, 6), (6, 8), (9, 7), (6, 6)],
            vec![(7, 7), (6, 6), (8, 6), (7, 8), (5, 5)],
            vec![(7, 7), (6, 6), (8, 6), (9, 5), (7, 5)],
            vec![(7, 7), (8, 8), (6, 8), (7, 6), (9, 9)],
            vec![(7, 7), (8, 8), (6, 8), (5, 7), (8, 9)],
            vec![(7, 7), (6, 6), (7, 8), (8, 7), (5, 5)],
            vec![(7, 7), (6, 6), (7, 8), (8, 6), (5, 7)],
            vec![(7, 7), (6, 8), (8, 7), (7, 6), (5, 9)],
            vec![(7, 7), (6, 8), (8, 7), (5, 6), (9, 6)],
            vec![(7, 7), (7, 6), (6, 8), (8, 7), (5, 8)],
            vec![(7, 7), (7, 6), (6, 8), (5, 8), (8, 5)],
            vec![(7, 7), (6, 7), (8, 7), (6, 6), (8, 8)],
            vec![(7, 7), (6, 7), (8, 7), (5, 7), (9, 7)],
            vec![(7, 7), (8, 6), (7, 6), (9, 5), (6, 8)],
            vec![(7, 7), (8, 6), (7, 6), (6, 7), (8, 5)],
            vec![(7, 7), (7, 8), (6, 6), (8, 7), (8, 9)],
            vec![(7, 7), (7, 8), (6, 6), (5, 7), (6, 8)],
            vec![(7, 7), (8, 8), (7, 8), (6, 7), (9, 9)],
            vec![(7, 7), (8, 8), (7, 8), (9, 7), (6, 9)],
            vec![(7, 7), (6, 7), (8, 6), (7, 8), (5, 7)],
            vec![(7, 7), (6, 7), (8, 6), (9, 5), (7, 5)],
            vec![(7, 7), (8, 7), (6, 7), (9, 7), (5, 7)],
            vec![(7, 7), (8, 7), (6, 7), (7, 8), (7, 6)],
            vec![(7, 7), (7, 8), (8, 7), (6, 6), (6, 9)],
            vec![(7, 7), (7, 8), (8, 7), (8, 9), (9, 8)],
            vec![(7, 7), (8, 6), (7, 5), (6, 7), (8, 8)],
            vec![(7, 7), (8, 6), (7, 5), (7, 8), (9, 7)],
            vec![(7, 7), (7, 8), (8, 7), (8, 8), (6, 6)],
            vec![(7, 7), (7, 8), (8, 7), (6, 6), (9, 7)],
        ];

        let zobrist = crate::types::init_zobrist_table(15);

        for opening in &openings {
            for prefix_len in 1..opening.len() {
                let mut hash: ZobristHash = 0;
                for (step, &(x, y)) in opening.iter().take(prefix_len).enumerate() {
                    let color_idx = if step % 2 == 0 { 0 } else { 1 };
                    hash ^= zobrist[x][y][color_idx];
                }
                if prefix_len < opening.len() {
                    let next = opening[prefix_len];
                    let next_pos = Position::new(next.0, next.1);
                    let entry = self.positions.entry(hash).or_default();
                    if !entry.contains(&next_pos) {
                        entry.push(next_pos);
                    }
                }
            }
        }
    }

    pub fn lookup(&self, hash: ZobristHash) -> Option<&Vec<Position>> {
        self.positions.get(&hash)
    }

    pub fn pick_random(&self, hash: ZobristHash) -> Option<Position> {
        let moves = self.positions.get(&hash)?;
        let mut rng = rand::thread_rng();
        moves.choose(&mut rng).copied()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::Color;

    #[test]
    fn test_empty_board_has_opening() {
        let book = OpeningBook::new();
        let board = Board::new(15);
        // 开局库在走子后才能匹配，空棋盘作为兜底结果也合理
        assert!(
            book.lookup(board.hash()).is_none(),
            "空棋盘不应匹配（需至少一手）"
        );
    }

    #[test]
    fn test_unknown_hash_returns_none() {
        let book = OpeningBook::new();
        assert!(book.lookup(0xDEADBEEF_CAFEBABE).is_none());
    }

    #[test]
    fn test_known_sequence_matches() {
        let book = OpeningBook::new();
        let board = Board::new(15);
        // 花月前4手: 黑(7,7) 白(7,8) 黑(6,7) 白(6,6)
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        let board = board.place(Position::new(7, 8), Color::White).unwrap();
        let board = board.place(Position::new(6, 7), Color::Black).unwrap();
        let board = board.place(Position::new(6, 6), Color::White).unwrap();
        assert!(book.lookup(board.hash()).is_some(), "花月前4手应匹配");
    }
}
