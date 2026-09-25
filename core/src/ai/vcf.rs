use crate::board::Board;
use crate::rules;
use crate::scan;
use crate::types::{CellState, Color, Position};

/// VCF 搜索 — 连续冲四取胜。返回取胜序列第一步
pub fn vcf_search(board: &Board, color: Color, max_depth: usize) -> Option<Position> {
    vcf_inner(board, color, max_depth).map(|seq| seq[0])
}

fn vcf_inner(board: &Board, color: Color, depth: usize) -> Option<Vec<Position>> {
    if depth == 0 {
        return None;
    }
    let candidates = board.get_candidate_moves();
    for &pos in &candidates {
        if rules::is_forbidden(board, pos, color) {
            continue;
        }
        if let Ok(new_board) = board.place(pos, color) {
            if new_board.check_win(pos) {
                return Some(vec![pos]);
            }
            if is_rush_four(&new_board, pos, color) {
                let opp = color.opponent();
                if let Some(block) = find_unique_block(&new_board, pos, color) {
                    if let Ok(b2) = new_board.place(block, opp) {
                        if let Some(mut rest) = vcf_inner(&b2, color, depth - 2) {
                            rest.insert(0, pos);
                            return Some(rest);
                        }
                    }
                }
            }
        }
    }
    None
}

/// VCT 搜索 — 连续活三/冲四混合取胜
pub fn vct_search(board: &Board, color: Color, max_depth: usize) -> Option<Position> {
    vct_inner(board, color, max_depth).map(|seq| seq[0])
}

fn vct_inner(board: &Board, color: Color, depth: usize) -> Option<Vec<Position>> {
    if depth == 0 {
        return None;
    }
    let candidates = board.get_candidate_moves();
    for &pos in &candidates {
        if rules::is_forbidden(board, pos, color) {
            continue;
        }
        if let Ok(new_board) = board.place(pos, color) {
            if new_board.check_win(pos) {
                return Some(vec![pos]);
            }
            if is_threat(&new_board, pos, color) {
                let opp = color.opponent();
                let defenses = find_threat_defenses(&new_board, pos, color);
                if defenses.len() == 1 {
                    if let Ok(b2) = new_board.place(defenses[0], opp) {
                        if let Some(mut rest) = vct_inner(&b2, color, depth - 2) {
                            rest.insert(0, pos);
                            return Some(rest);
                        }
                    }
                }
            }
        }
    }
    None
}

fn is_rush_four(board: &Board, pos: Position, color: Color) -> bool {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let info = scan::scan_direction(board, pos, color, dx, dy);
        if info.count == 4
            && (info.start_open || info.end_open)
            && !(info.start_open && info.end_open)
        {
            return true;
        }
    }
    false
}

fn is_threat(board: &Board, pos: Position, color: Color) -> bool {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let info = scan::scan_direction(board, pos, color, dx, dy);
        if (info.count == 3 && info.start_open && info.end_open)
            || (info.count == 4 && (info.start_open || info.end_open))
        {
            return true;
        }
    }
    false
}

fn find_unique_block(board: &Board, pos: Position, color: Color) -> Option<Position> {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let info = scan::scan_direction(board, pos, color, dx, dy);
        if info.count == 4 {
            if info.start_open {
                let nx = pos.x as isize - dx * (info.count as isize - 1) - dx;
                let ny = pos.y as isize - dy * (info.count as isize - 1) - dy;
                if let Some(CellState::Empty) = scan::cell_at(board, nx, ny) {
                    return Some(Position::new(nx as usize, ny as usize));
                }
            }
            if info.end_open {
                let nx = pos.x as isize + dx * (info.count as isize - 1) + dx;
                let ny = pos.y as isize + dy * (info.count as isize - 1) + dy;
                if let Some(CellState::Empty) = scan::cell_at(board, nx, ny) {
                    return Some(Position::new(nx as usize, ny as usize));
                }
            }
        }
    }
    None
}

fn find_threat_defenses(board: &Board, pos: Position, color: Color) -> Vec<Position> {
    let mut defenses = Vec::new();
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let info = scan::scan_direction(board, pos, color, dx, dy);
        if info.count >= 3 {
            if info.start_open {
                let sx = pos.x as isize - dx * info.count as isize;
                let sy = pos.y as isize - dy * info.count as isize;
                if let Some(CellState::Empty) = scan::cell_at(board, sx, sy) {
                    defenses.push(Position::new(sx as usize, sy as usize));
                }
            }
            let ex = pos.x as isize + dx * info.count as isize;
            let ey = pos.y as isize + dy * info.count as isize;
            if let Some(CellState::Empty) = scan::cell_at(board, ex, ey) {
                defenses.push(Position::new(ex as usize, ey as usize));
            }
        }
    }
    defenses.sort();
    defenses.dedup();
    defenses
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::board::Board;
    use crate::types::Color;

    #[test]
    fn test_vcf_empty_board_returns_none() {
        let board = Board::new(15);
        assert!(vcf_search(&board, Color::Black, 6).is_none());
    }

    #[test]
    fn test_vct_empty_board_returns_none() {
        let board = Board::new(15);
        let board = board.place(Position::new(7, 7), Color::Black).unwrap();
        assert!(vct_search(&board, Color::Black, 6).is_none());
    }

    #[test]
    fn test_vcf_detects_rush_four() {
        let board = Board::new(15);
        let mut board = board;
        board = board.place(Position::new(7, 3), Color::Black).unwrap();
        board = board.place(Position::new(7, 4), Color::Black).unwrap();
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        let result = vcf_search(&board, Color::Black, 4);
        assert!(result.is_some());
    }
}
