use crate::ai::evaluate::evaluate_board;
use crate::ai::killer::KillerTable;
use crate::ai::opening::OpeningBook;
use crate::ai::trans_table::{BoundType, TransTable};
use crate::ai::vcf;
use crate::ai::AiEngine;
use crate::board::Board;
use crate::rules;
use crate::types::{Color, Position};
use std::time::{Duration, Instant};

const TIME_LIMITS: [u64; 5] = [1, 2, 3, 5, 8];

#[derive(Clone)]
pub struct AlphaBetaAi {
    difficulty: usize,
}

impl AlphaBetaAi {
    pub fn new(difficulty: usize) -> Self {
        Self { difficulty }
    }

    fn time_limit(&self) -> Duration {
        let idx = self.difficulty.saturating_sub(1).min(4);
        Duration::from_secs(TIME_LIMITS[idx])
    }
}

impl AiEngine for AlphaBetaAi {
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        // 1. 开局库（前 7 手）
        if board.history().len() < 7 {
            let book = OpeningBook::new();
            if let Some(pos) = book.pick_random(board.hash()) {
                return Some(pos);
            }
        }

        // 2. VCF/VCT 浅搜索
        if let Some(pos) = vcf::vcf_search(board, color, 6) {
            return Some(pos);
        }
        if let Some(pos) = vcf::vct_search(board, color, 8) {
            return Some(pos);
        }

        // 3. 迭代加深 Alpha-Beta
        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return None;
        }

        let start = Instant::now();
        let time_limit = self.time_limit();
        let mut best_pos = candidates[0];
        let mut tt = TransTable::new();
        let mut killer = KillerTable::new();

        for depth in 1..=20u32 {
            if start.elapsed() >= time_limit {
                break;
            }

            let (pos, completed) =
                self.search_depth(board, color, depth, &mut tt, &mut killer, start, time_limit);

            if let Some(p) = pos {
                best_pos = p;
            }

            if !completed {
                break;
            }
        }

        Some(best_pos)
    }
}

impl AlphaBetaAi {
    #[allow(clippy::too_many_arguments)]
    fn search_depth(
        &self,
        board: &Board,
        color: Color,
        depth: u32,
        tt: &mut TransTable,
        killer: &mut KillerTable,
        start: Instant,
        time_limit: Duration,
    ) -> (Option<Position>, bool) {
        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return (None, true);
        }

        let mut best_pos = None;
        let mut best_score = f64::NEG_INFINITY;
        let mut alpha = f64::NEG_INFINITY;
        let beta = f64::INFINITY;
        let mut completed = true;

        // 启发式排序: killer + 立即五连 + evaluate
        let killer_moves = killer.get(depth as usize);
        let mut scored: Vec<(Position, f64)> = candidates
            .iter()
            .filter(|&&p| !rules::is_forbidden(board, p, color))
            .filter_map(|&p| {
                board.place(p, color).ok().map(|b| {
                    if b.check_win(p) {
                        (p, f64::INFINITY)
                    } else {
                        (p, evaluate_board(&b, color))
                    }
                })
            })
            .collect();

        scored.sort_by(|a, b| {
            let a_k = killer_moves.contains(&Some(a.0));
            let b_k = killer_moves.contains(&Some(b.0));
            if a_k && !b_k {
                std::cmp::Ordering::Less
            } else if !a_k && b_k {
                std::cmp::Ordering::Greater
            } else {
                b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal)
            }
        });

        for (pos, _) in scored {
            if start.elapsed() >= time_limit {
                completed = false;
                break;
            }

            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    return (Some(pos), true);
                }
                let score = -self.negamax(
                    &new_board,
                    depth - 1,
                    -beta,
                    -alpha,
                    color.opponent(),
                    tt,
                    killer,
                    start,
                    time_limit,
                );
                if score > best_score {
                    best_score = score;
                    best_pos = Some(pos);
                }
                if score > alpha {
                    alpha = score;
                }
            }
        }

        (best_pos, completed)
    }

    #[allow(clippy::too_many_arguments)]
    fn negamax(
        &self,
        board: &Board,
        depth: u32,
        mut alpha: f64,
        beta: f64,
        color: Color,
        tt: &mut TransTable,
        killer: &mut KillerTable,
        start: Instant,
        time_limit: Duration,
    ) -> f64 {
        if start.elapsed() >= time_limit {
            return evaluate_board(board, color);
        }

        // 置换表
        let hash = board.hash();
        let alpha_orig = alpha;
        if let Some(entry) = tt.probe(hash, depth as u8) {
            match entry.bound {
                BoundType::Exact => return entry.score as f64,
                BoundType::LowerBound => alpha = alpha.max(entry.score as f64),
                BoundType::UpperBound => {
                    if (entry.score as f64) <= alpha {
                        return entry.score as f64;
                    }
                }
            }
            if alpha >= beta {
                return entry.score as f64;
            }
        }

        if depth == 0 {
            return evaluate_board(board, color);
        }

        let candidates = board.get_candidate_moves();
        if candidates.is_empty() {
            return evaluate_board(board, color);
        }

        // 启发式排序
        let killer_moves = killer.get(depth as usize);
        let mut scored: Vec<(Position, f64)> = candidates
            .into_iter()
            .filter(|&p| !rules::is_forbidden(board, p, color))
            .filter_map(|p| {
                board.place(p, color).ok().map(|b| {
                    if b.check_win(p) {
                        (p, f64::INFINITY)
                    } else {
                        (p, evaluate_board(&b, color))
                    }
                })
            })
            .collect();

        scored.sort_by(|a, b| {
            let a_k = killer_moves.contains(&Some(a.0));
            let b_k = killer_moves.contains(&Some(b.0));
            if a_k && !b_k {
                std::cmp::Ordering::Less
            } else if !a_k && b_k {
                std::cmp::Ordering::Greater
            } else {
                b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal)
            }
        });

        let mut max_val = f64::NEG_INFINITY;
        let mut best_move = None;

        for (pos, _) in scored {
            if start.elapsed() >= time_limit {
                break;
            }

            if let Ok(new_board) = board.place(pos, color) {
                if new_board.check_win(pos) {
                    tt.store(
                        hash,
                        depth as u8,
                        f64::INFINITY as i32,
                        BoundType::Exact,
                        Some(pos),
                    );
                    return f64::INFINITY;
                }
                let val = -self.negamax(
                    &new_board,
                    depth - 1,
                    -beta,
                    -alpha,
                    color.opponent(),
                    tt,
                    killer,
                    start,
                    time_limit,
                );
                if val > max_val {
                    max_val = val;
                    best_move = Some(pos);
                }
                if val > alpha {
                    alpha = val;
                }
                if alpha >= beta {
                    killer.record(depth as usize, pos);
                    break;
                }
            }
        }

        let bound = if max_val <= alpha_orig {
            BoundType::UpperBound
        } else if max_val >= beta {
            BoundType::LowerBound
        } else {
            BoundType::Exact
        };
        tt.store(hash, depth as u8, max_val as i32, bound, best_move);

        max_val
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::{Color, Position};

    #[test]
    fn test_time_limits() {
        assert_eq!(AlphaBetaAi::new(1).time_limit(), Duration::from_secs(1));
        assert_eq!(AlphaBetaAi::new(5).time_limit(), Duration::from_secs(8));
    }

    #[test]
    fn test_ai_returns_move_on_empty_board() {
        let board = Board::new(15);
        let ai = AlphaBetaAi::new(3);
        let mv = ai.best_move(&board, Color::Black);
        assert!(mv.is_some());
    }

    #[test]
    fn test_ai_takes_winning_move() {
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
            "AI should win, got ({},{})",
            mv.x,
            mv.y
        );
    }
}
