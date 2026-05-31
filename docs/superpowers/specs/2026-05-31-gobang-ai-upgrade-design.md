# Gobang AI 升级设计文档

> 状态: 待审核 | 日期: 2026-05-31

## 目标

将当前基础 Alpha-Beta AI 升级为专业级五子棋 AI，具备迭代加深、置换表、组合棋形评估、VCF/VCT 杀棋搜索和开局库。

## 架构变更

```
当前:                             升级后:
AlphaBetaAi                       AlphaBetaAi
  └─ negamax()                      ├─ iterative_deepening()  ← 新增：逐层加深 + 时间控制
                                    ├─ negamax()              ← 改进：加 TT + killer
                                    │    ├─ evaluate_board()  ← 改进：组合棋形 + 位置权重
                                    │    └─ tt: TransTable    ← 新增：置换表缓存
                                    ├─ vcf_search()           ← 新增：连续冲四取胜搜索
                                    ├─ vct_search()           ← 新增：连续活三取胜搜索
                                    └─ opening_book           ← 新增：开局定式库
```

---

## 一、迭代加深 + 时间控制

### 原理
不再固定搜索 depth=N，而是从 depth=1 开始，每轮加深 1 层，直到时间用尽。每轮完成后保存 best_move，超时时返回上一轮的结果。

### 实现
```
best_move(board, color):
    start_time = now()
    time_limit = 根据难度映射: [1,2,3,5,8] 秒 → level 1~5
    best = center_position

    for depth in 1..=MAX_DEPTH:
        result = negamax_root(board, depth, color)
        if 搜索完成没超时:
            best = result.best_move
        else:
            break  // 超时，返回上轮 best

    return best
```

### 时间分配策略
- 每层完成后检查是否剩余时间 < 本层耗时 × 1.5，如果是则不再加深
- 防止"刚开搜就超时"的无效搜索

### 难度→时间映射
| Level | 时间上限 | depth 上限 |
|-------|---------|-----------|
| 1 | 1s | 4 |
| 2 | 2s | 6 |
| 3 | 3s | 8 |
| 4 | 5s | 12 |
| 5 | 8s | 20 |

---

## 二、置换表 (Transposition Table)

### 原理
五子棋中不同走子顺序可能到达同一局面。用 Zobrist 哈希为每个局面生成唯一 key，缓存搜索结果。下次遇到相同哈希直接查表，避免重复搜索。

### 数据结构
```rust
struct TransTable {
    entries: Vec<Option<TTEntry>>,  // 2^N 大小，N=20 → 约 100 万条目
    size_mask: u64,
}

struct TTEntry {
    hash: u64,           // 完整哈希（防冲突）
    depth: u8,           // 搜索深度
    score: i32,          // 局面评分 (转为整数)
    bound: BoundType,    // Exact / LowerBound / UpperBound
    best_move: Option<Position>,
}
```

### Zobrist 哈希
```rust
// 初始化：全局二维随机数表
// zobrist[color][x][y] = random_u64()
// 局面哈希 = XOR 所有棋子的 zobrist[color][x][y]
// 落子时增量更新：hash ^= zobrist[color][x][y]
// 不提子所以不需要 undo 操作，直接用 hash ^=
```

### 查表/存表
```
negamax(board, depth, alpha, beta, color):
    hash = board.zobrist_hash
    // 查表
    if let Some(entry) = tt.probe(hash, depth):
        if entry.depth >= depth:
            match entry.bound:
                Exact    => return entry.score
                LowerBound => alpha = max(alpha, entry.score)
                UpperBound => beta  = min(beta,  entry.score)
            if alpha >= beta: return entry.score

    // ... 正常搜索 ...

    // 存表
    if score <= alpha_orig: bound = UpperBound
    elif score >= beta:     bound = LowerBound
    else:                   bound = Exact
    tt.store(hash, depth, score, bound, best_move)
```

### 配置
- 表大小：2^20 条目 ≈ 32MB（每条目 ~32 bytes）
- 替换策略：深度优先（depth 深的覆盖 depth 浅的）

---

## 三、组合棋形评估

### 问题
当前评估单方向扫描，无法识别"一个方向活三 + 垂直方向活三 = 必胜威胁"的组合。

### 改进：多方向特征向量

```rust
struct PositionFeatures {
    // 四个方向 (水平、垂直、对角线1、对角线2) 各自的最大棋形
    max_pattern: [Pattern; 4],       // Five/OpenFour/RushFour/OpenThree/...
    combo_score: f64,                // 组合加分
    position_bonus: f64,             // 位置权重
}

enum Pattern {
    Five, OpenFour, RushFour, OpenThree, SleepThree,
    OpenTwo, SleepTwo, OpenOne, Empty,
}
```

### 组合评分规则
| 组合 | 加分 | 说明 |
|------|------|------|
| 活三 + 活三 (交叉方向) | 5000 | 必胜形 |
| 活三 + 冲四 | 10000 | 近似必胜 |
| 冲四 + 冲四 | 8000 | 双重威胁 |
| 活三 + 活二 | 500 | 发展优势 |

### 位置权重
```
position_score(x, y, board_size) =
    基础距离分: 离中心越近越高 (高斯分布)
    + 边缘惩罚: 边缘 2 行内下降 50%
    + 星位偏好: 标准星位额外 +5%
```

位置权重占总评分的 ~5%，主要影响开局和中盘。

---

## 四、杀棋启发 (Killer Move Heuristic)

### 原理
记录每层深度中触发 Beta 剪枝的走法。同一层深的相似局面，上次有效的走法这次也优先搜索。

### 数据结构
```rust
// 每层存 2 个 killer move
killer_moves: [[Option<Position>; 2]; MAX_DEPTH]
```

### 候选排序优先级
1. 置换表中的 best_move
2. killer_moves[depth]
3. 能立即五连的走法
4. evaluate_board 预评分排序的其余走法

---

## 五、VCF/VCT 杀棋搜索

### VCF (Victory by Continuous Fours)
连续冲四取胜。当一方有冲四时，对手必须堵，我方继续冲四，直到五连。

```
vcf_search(board, color, depth_limit=10):
    if check_win: return Some(win_path)

    for each rush_four position for `color`:
        board.place(pos)
        opponent_blocks = forced_block_positions(board)  // 对手只有一种堵法
        if len(opponent_blocks) == 1:
            board.place(opponent_block)   // 对手被迫堵
            result = vcf_search(board, color, depth-2)
            if result: return Some(pos + result)
        board.undo(2)

    return None  // 无必胜序列
```

### VCT (Victory by Continuous Threats)
类似 VCF 但目标棋形更宽（冲四 + 活三），搜索更深。

- VCF：仅搜索连续冲四序列，depth ≤ 10
- VCT：搜索冲四/活三混合序列，depth ≤ 15

### 触发时机
在 `best_move` 中：
1. 先跑 VCF/VCT 浅搜索（depth=6）
2. 如果找到必胜序列 → 直接返回第一步
3. 否则 → 正常 Alpha-Beta 搜索
4. 如果 AB 搜索发现对手有威胁 → 防御模式，优先堵 VCF/VCT 路径

---

## 六、开局库

### 格式
```rust
struct OpeningBook {
    // hash → [best_moves]
    positions: HashMap<u64, Vec<Position>>,
}

// 初始化时从内置数据加载
fn load_opening_book() -> OpeningBook {
    // 内置 50 个常见开局定式
    // 花月、浦月、云月、雨月、溪月、金星、水月、新月 ...
}
```

### 数据来源
内置 50 个标准五子棋开局定式，覆盖前 3~7 手。直接从专业棋谱提取坐标序列，编译期嵌入。

### 使用逻辑
```
best_move():
    if 总手数 < opening_book_threshold:  // 前 7 手
        if let Some(moves) = opening_book.lookup(board.hash):
            return random_choice(moves)  // 从定式中随机选一个变招

    // 超过开局阶段，正常搜索
    return iterative_deepening(...)
```

---

## 七、文件变更

| 文件 | 操作 | 内容 |
|------|------|------|
| `core/Cargo.toml` | 改 | 加 `rand` 依赖（开局随机选择），加 `fxhash`（快速哈希） |
| `core/src/ai/mod.rs` | 改 | AiEngine trait 不变 |
| `core/src/ai/search.rs` | 重写 | 迭代加深 + TT + killer + VCF/VCT 入口 |
| `core/src/ai/evaluate.rs` | 重写 | 组合棋形 + 位置权重 |
| `core/src/ai/trans_table.rs` | 新建 | 置换表实现 (Zobrist + HashMap) |
| `core/src/ai/killer.rs` | 新建 | Killer move 表 |
| `core/src/ai/vcf.rs` | 新建 | VCF/VCT 杀棋搜索 |
| `core/src/ai/opening.rs` | 新建 | 开局库 (50 定式) |
| `core/src/board.rs` | 改 | Board 加 zobrist_hash 字段，place/undo 增量更新 |
| `core/src/types.rs` | 改 | 加 Zobrist 相关类型 |
| `gui/src/commands.rs` | 改 | new_game 适配新的 AI 参数（时间上限替代 depth） |

---

## 八、测试策略

| 模块 | 测试 |
|------|------|
| Zobrist 哈希 | 落子后哈希变化、对称局面哈希不同、undo 后恢复 |
| 置换表 | 存/查/同局面命中、depth 优先级替换 |
| 组合棋形 | 单方向评分不变、交叉活三加分、冲四+活三加分 |
| 位置权重 | 中心>边缘、对称位置权重相同 |
| Killer | 插入、查询、满容量替换 |
| VCF | 已知必胜序列被找到、无解返回 None |
| 开局库 | lookup 已知局面、未知局面返回 None |
| 迭代加深 | 超时返回有效 move、时间限制内完成 |
| AI 回归 | 原有 3 个 AI 测试仍然通过 |

---

## 九、不做 (YAGNI)

- 多线程并行搜索（收益有限，复杂度高）
- 蒙特卡洛树搜索（五子棋不适合）
- 神经网络评估（太重）
- 在线开局库更新
- 残局库
