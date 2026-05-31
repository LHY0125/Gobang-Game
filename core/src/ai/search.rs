use crate::ai::evaluate::evaluate_board;
use crate::ai::AiEngine;
use crate::board::Board;
use crate::rules;
use crate::types::{Color, Position};

/// Alpha-Beta AI 引擎
#[derive(Clone)]
pub struct AlphaBetaAi {
    depth: usize,
}

impl AlphaBetaAi {
    pub fn new(depth: usize) -> Self {
        Self { depth }
    }
}

impl AiEngine for AlphaBetaAi {
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return None;
        }

        let mut best_pos = None;
        let mut best_score = f64::NEG_INFINITY;

        for &pos in &candidates {
            // 禁手检查: 黑棋不能走禁手位置
            if rules::is_forbidden(board, pos, color) {
                continue;
            }
            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    return Some(pos);
                }
                let score = -self.negamax(
                    &new_board,
                    self.depth - 1,
                    f64::NEG_INFINITY,
                    f64::INFINITY,
                    color.opponent(),
                );
                if score > best_score {
                    best_score = score;
                    best_pos = Some(pos);
                }
            }
        }

        best_pos
    }
}

impl AlphaBetaAi {
    fn negamax(&self, board: &Board, depth: usize, mut alpha: f64, beta: f64, color: Color) -> f64 {
        if depth == 0 {
            return evaluate_board(board, color);
        }

        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return evaluate_board(board, color);
        }

        // 启发式排序：先评估每步棋，优先搜索高分走法 (跳过禁手)
        let mut scored: Vec<(Position, f64)> = candidates
            .into_iter()
            .filter(|&pos| !rules::is_forbidden(board, pos, color))
            .filter_map(|pos| {
                board.place(pos, color).ok().map(|b| {
                    if b.check_win(pos) {
                        (pos, f64::INFINITY)
                    } else {
                        let s = evaluate_board(&b, color);
                        (pos, s)
                    }
                })
            })
            .collect();
        scored.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));

        let mut max_val = f64::NEG_INFINITY;
        for (pos, _) in scored {
            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    return f64::INFINITY;
                }
                let val = -self.negamax(&new_board, depth - 1, -beta, -alpha, color.opponent());
                if val > max_val {
                    max_val = val;
                }
                if val > alpha {
                    alpha = val;
                }
                if alpha >= beta {
                    break;
                }
            }
        }
        max_val
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ai::AiEngine;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_ai_returns_center_on_empty_board() {
        let board = Board::new(15);
        let ai = AlphaBetaAi::new(1);
        let mv = ai.best_move(&board, Color::Black);
        assert!(mv.is_some());
        let pos = mv.unwrap();
        assert!(pos.x >= 6 && pos.x <= 8);
        assert!(pos.y >= 6 && pos.y <= 8);
    }

    #[test]
    fn test_ai_blocks_rush_four() {
        // 白棋活三 (一端被己方黑棋堵住, 只有一端开放)
        let board = Board::new(15);
        let mut board = board;
        board = board.place(Position::new(7, 1), Color::Black).unwrap();
        board = board.place(Position::new(7, 2), Color::White).unwrap();
        board = board.place(Position::new(7, 3), Color::White).unwrap();
        board = board.place(Position::new(7, 4), Color::White).unwrap();
        board = board.place(Position::new(7, 5), Color::White).unwrap();
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black).unwrap();
        assert_eq!(
            mv,
            Position::new(7, 6),
            "AI should block rush four at (7,6), got ({},{})",
            mv.x,
            mv.y
        );
    }

    #[test]
    fn test_ai_blocks_four_near_edge() {
        // 白棋冲四 (靠边), 黑棋只需堵住开放端
        let board = Board::new(15);
        let mut board = board;
        board = board.place(Position::new(7, 0), Color::White).unwrap();
        board = board.place(Position::new(7, 1), Color::White).unwrap();
        board = board.place(Position::new(7, 2), Color::White).unwrap();
        board = board.place(Position::new(7, 3), Color::White).unwrap();
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black).unwrap();
        assert_eq!(
            mv,
            Position::new(7, 4),
            "AI should block four at (7,4), got ({},{})",
            mv.x,
            mv.y
        );
    }

    #[test]
    fn test_ai_takes_win() {
        // 黑棋连四, (7,2) 和 (7,7) 都是胜着
        let board = Board::new(15);
        let mut board = board;
        board = board.place(Position::new(7, 3), Color::Black).unwrap();
        board = board.place(Position::new(7, 4), Color::Black).unwrap();
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black).unwrap();
        assert!(
            (mv.x == 7 && mv.y == 2) || (mv.x == 7 && mv.y == 7),
            "AI should take winning move, got ({},{})",
            mv.x,
            mv.y
        );
    }
}
