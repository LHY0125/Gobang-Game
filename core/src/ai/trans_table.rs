use crate::types::{Position, ZobristHash};

const TT_SIZE: usize = 1 << 20; // ~100 万条目
const TT_MASK: usize = TT_SIZE - 1;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BoundType {
    Exact,
    LowerBound,
    UpperBound,
}

#[derive(Debug, Clone)]
pub struct TTEntry {
    pub hash: ZobristHash,
    pub depth: u8,
    pub score: i32,
    pub bound: BoundType,
    pub best_move: Option<Position>,
}

pub struct TransTable {
    entries: Box<[Option<TTEntry>]>,
}

impl TransTable {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn probe(&self, hash: ZobristHash, depth: u8) -> Option<&TTEntry> {
        let idx = (hash as usize) & TT_MASK;
        self.entries[idx]
            .as_ref()
            .filter(|e| e.hash == hash && e.depth >= depth)
    }

    pub fn store(
        &mut self,
        hash: ZobristHash,
        depth: u8,
        score: i32,
        bound: BoundType,
        best_move: Option<Position>,
    ) {
        let idx = (hash as usize) & TT_MASK;
        let should_replace = match &self.entries[idx] {
            None => true,
            Some(old) => depth >= old.depth,
        };
        if should_replace {
            self.entries[idx] = Some(TTEntry {
                hash,
                depth,
                score,
                bound,
                best_move,
            });
        }
    }

    pub fn clear(&mut self) {
        for entry in self.entries.iter_mut() {
            *entry = None;
        }
    }
}

impl Default for TransTable {
    fn default() -> Self {
        Self {
            entries: vec![None; TT_SIZE].into_boxed_slice(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_store_and_probe() {
        let mut tt = TransTable::new();
        tt.store(12345, 3, 100, BoundType::Exact, Some(Position::new(7, 7)));
        let entry = tt.probe(12345, 2).unwrap();
        assert_eq!(entry.score, 100);
        assert_eq!(entry.best_move, Some(Position::new(7, 7)));
    }

    #[test]
    fn test_depth_requirement() {
        let mut tt = TransTable::new();
        tt.store(42, 5, 200, BoundType::Exact, None);
        assert!(tt.probe(42, 4).is_some());
        assert!(tt.probe(42, 6).is_none());
    }

    #[test]
    fn test_hash_collision_prevention() {
        let mut tt = TransTable::new();
        tt.store(100, 3, 50, BoundType::Exact, None);
        assert!(tt.probe(200, 1).is_none());
    }

    #[test]
    fn test_depth_priority_replacement() {
        let mut tt = TransTable::new();
        tt.store(999, 2, 10, BoundType::Exact, None);
        tt.store(999, 5, 99, BoundType::Exact, None);
        assert_eq!(tt.probe(999, 3).unwrap().score, 99);
    }

    #[test]
    fn test_clear() {
        let mut tt = TransTable::new();
        tt.store(1, 1, 1, BoundType::Exact, None);
        tt.clear();
        assert!(tt.probe(1, 0).is_none());
    }
}
