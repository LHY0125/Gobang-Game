use crate::board::Board;
use crate::rules;
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
        let (count, start_open, end_open) = scan_vcf(board, pos, color, dx, dy);
        if count == 4 && (start_open || end_open) && !(start_open && end_open) {
            return true;
        }
    }
    false
}

fn is_threat(board: &Board, pos: Position, color: Color) -> bool {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let (count, start_open, end_open) = scan_vcf(board, pos, color, dx, dy);
        if (count == 3 && start_open && end_open) || (count == 4 && (start_open || end_open)) {
            return true;
        }
    }
    false
}

fn find_unique_block(board: &Board, pos: Position, color: Color) -> Option<Position> {
    let directions: [(isize, isize); 4] = [(0, 1), (1, 0), (1, 1), (1, -1)];
    for (dx, dy) in directions {
        let (count, start_open, end_open) = scan_vcf(board, pos, color, dx, dy);
        if count == 4 {
            if start_open {
                // 扫描开放端找到空位
                let nx = pos.x as isize - dx * count as isize;
                let ny = pos.y as isize - dy * count as isize;
                if nx >= 0
                    && ny >= 0
                    && (nx as usize) < board.size
                    && (ny as usize) < board.size
                    && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty
                {
                    return Some(Position::new(nx as usize, ny as usize));
                }
            }
            if end_open {
                let nx = pos.x as isize + dx * count as isize;
                let ny = pos.y as isize + dy * count as isize;
                if nx >= 0
                    && ny >= 0
                    && (nx as usize) < board.size
                    && (ny as usize) < board.size
                    && board.get(Position::new(nx as usize, ny as usize)) == CellState::Empty
                {
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
        let (count, start_open, _end_open) = scan_vcf(board, pos, color, dx, dy);
        if count >= 3 {
            if start_open {
                let sx = pos.x as isize - dx * count as isize;
                let sy = pos.y as isize - dy * count as isize;
                if sx >= 0 && sy >= 0 && (sx as usize) < board.size && (sy as usize) < board.size {
                    defenses.push(Position::new(sx as usize, sy as usize));
                }
            }
            let ex = pos.x as isize + dx * count as isize;
            let ey = pos.y as isize + dy * count as isize;
            if ex >= 0 && ey >= 0 && (ex as usize) < board.size && (ey as usize) < board.size {
                defenses.push(Position::new(ex as usize, ey as usize));
            }
        }
    }
    defenses.sort();
    defenses.dedup();
    defenses
}

fn scan_vcf(board: &Board, pos: Position, color: Color, dx: isize, dy: isize) -> (u32, bool, bool) {
    let mut count = 1u32;

    let mut nx = pos.x as isize + dx;
    let mut ny = pos.y as isize + dy;
    while let Some(cell) = cell_at(board, nx, ny) {
        if cell == CellState::Occupied(color) {
            count += 1;
        } else {
            break;
        }
        nx += dx;
        ny += dy;
    }
    let end_open = cell_at(board, nx, ny) == Some(CellState::Empty);

    let mut nx = pos.x as isize - dx;
    let mut ny = pos.y as isize - dy;
    while let Some(cell) = cell_at(board, nx, ny) {
        if cell == CellState::Occupied(color) {
            count += 1;
        } else {
            break;
        }
        nx -= dx;
        ny -= dy;
    }
    let start_open = cell_at(board, nx, ny) == Some(CellState::Empty);

    (count, start_open, end_open)
}

fn cell_at(board: &Board, x: isize, y: isize) -> Option<CellState> {
    if x < 0 || y < 0 || (x as usize) >= board.size || (y as usize) >= board.size {
        return None;
    }
    Some(board.get(Position::new(x as usize, y as usize)))
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
        // 黑冲四: (7,3)(7,4)(7,5)(7,6) — 一端堵一端开放
        board = board.place(Position::new(7, 3), Color::Black).unwrap();
        board = board.place(Position::new(7, 4), Color::Black).unwrap();
        board = board.place(Position::new(7, 5), Color::Black).unwrap();
        board = board.place(Position::new(7, 6), Color::Black).unwrap();
        // 该局面是冲四，对手未堵
        let result = vcf_search(&board, Color::Black, 4);
        // 应该能找到直接五连（(7,7)或(7,2)），取决于哪个空
        assert!(result.is_some());
    }
}
