use crate::types::Position;

const MAX_DEPTH: usize = 32;
const SLOTS_PER_DEPTH: usize = 2;

pub struct KillerTable {
    moves: [[Option<Position>; SLOTS_PER_DEPTH]; MAX_DEPTH],
}

impl KillerTable {
    pub fn new() -> Self {
        Self {
            moves: [[None; SLOTS_PER_DEPTH]; MAX_DEPTH],
        }
    }

    /// 记录产生剪枝的走法，同一位置不会重复存储
    pub fn record(&mut self, depth: usize, pos: Position) {
        if depth >= MAX_DEPTH {
            return;
        }
        let slot0 = self.moves[depth][0];
        if slot0 != Some(pos) {
            self.moves[depth][1] = slot0;
            self.moves[depth][0] = Some(pos);
        }
    }

    /// 获取该深度的 killer moves（优先级: slot0 > slot1）
    pub fn get(&self, depth: usize) -> [Option<Position>; SLOTS_PER_DEPTH] {
        if depth >= MAX_DEPTH {
            return [None, None];
        }
        self.moves[depth]
    }

    pub fn clear(&mut self) {
        self.moves = [[None; SLOTS_PER_DEPTH]; MAX_DEPTH];
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_record_and_get() {
        let mut kt = KillerTable::new();
        kt.record(3, Position::new(7, 7));
        assert_eq!(kt.get(3)[0], Some(Position::new(7, 7)));
    }

    #[test]
    fn test_two_slots_eviction() {
        let mut kt = KillerTable::new();
        kt.record(1, Position::new(7, 7));
        kt.record(1, Position::new(8, 8));
        kt.record(1, Position::new(9, 9));
        let got = kt.get(1);
        assert_eq!(got[0], Some(Position::new(9, 9)));
        assert_eq!(got[1], Some(Position::new(8, 8)));
    }

    #[test]
    fn test_duplicate_not_reinserted() {
        let mut kt = KillerTable::new();
        kt.record(2, Position::new(7, 7));
        kt.record(2, Position::new(7, 7));
        assert_eq!(kt.get(2)[0], Some(Position::new(7, 7)));
        assert_eq!(kt.get(2)[1], None);
    }
}
