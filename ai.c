#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern int BOARD_SIZE;
extern int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
extern int step_count;
extern const int direction[4][2];
extern Step steps[MAX_STEPS];

/**
 * @brief 评估特定位置对当前玩家的战略价值。
 *        此函数通过模拟在该位置落子，然后分析形成的棋型来为该位置打分。
 *        分数越高，代表该位置对指定玩家越有利。
 * @param x 要评估的行坐标 (0-based)。
 * @param y 要评估的列坐标 (0-based)。
 * @param player 玩家标识 (PLAYER 或 AI)，代表为哪一方进行评估。
 * @return int 返回该位置的综合评估分数。
 * @note 评分系统设计：
 *  - 核心思想是为不同的棋型赋予不同的权重，棋型越接近胜利，权重越高。
 *  - “活”棋型（两端无阻挡）比“眠”棋型（一端有阻挡）或“死”棋型（两端被阻挡）得分高得多，
 *    因为它们有更大的发展潜力。
 *  - 评分标准 (仅为示例，可调整以优化AI行为):
 *    - 连五: 1,000,000 (胜利)
 *    - 活四: 100,000 (下一步胜利)
 *    - 冲四: 10,000 (下一步可能胜利)
 *    - 活三: 5,000 (潜力巨大)
 *    - 眠三: 1,000
 *    - 活二: 500
 *    - 眠二: 100
 *    - 其他: 更低的分数
 *  - 位置奖励：棋盘中心区域通常具有更高的战略价值，因此会给予额外加分，鼓励AI占据中心。
 */
int evaluate_pos(int x, int y, int player)
{
    // 保存原始值用于还原
    int original = board[x][y];
    // 模拟在该位置落子
    board[x][y] = player;

    int total_score = 0;      // 总分
    int line_scores[4] = {0}; // 四个方向的得分

    // 遍历四个方向进行评估
    for (int i = 0; i < 4; i++)
    {
        int dx = direction[i][0], dy = direction[i][1];
        // 获取当前方向上的棋型信息
        DirInfo info = count_specific_direction(x, y, dx, dy, player);

        // 直接形成五连珠为必胜
        if (info.continuous_chess >= 5)
        {
            board[x][y] = original; // 还原棋盘
            return 1000000;         // 返回最大分
        }

        // 根据连续棋子数评分
        switch (info.continuous_chess)
        {
        case 4:                                     // 四连珠
            if (info.check_start && info.check_end) // 活四(两端开放)
                line_scores[i] = 100000;
            else if (info.check_start || info.check_end) // 冲四(一端开放)
                line_scores[i] = 10000;
            else // 死四(两端封闭)
                line_scores[i] = 500;
            break;

        case 3:                                     // 三连珠
            if (info.check_start && info.check_end) // 活三
                line_scores[i] = 5000;
            else if (info.check_start || info.check_end) // 眠三
                line_scores[i] = 1000;
            else // 死三
                line_scores[i] = 50;
            break;

        case 2:                                     // 二连珠
            if (info.check_start && info.check_end) // 活二
                line_scores[i] = 500;
            else if (info.check_start || info.check_end) // 眠二
                line_scores[i] = 100;
            else // 死二
                line_scores[i] = 10;
            break;

        case 1:                                     // 单子
            if (info.check_start && info.check_end) // 开放位置
                line_scores[i] = 50;
            else if (info.check_start || info.check_end) // 半开放位置
                line_scores[i] = 10;
            else // 封闭位置
                line_scores[i] = 1;
            break;
        }
    }

    // 计算总分（最高方向分+其他方向分加权）
    int max_score = 0;
    int sum_score = 0;
    for (int i = 0; i < 4; i++)
    {
        if (line_scores[i] > max_score)
            max_score = line_scores[i];
        sum_score += line_scores[i];
    }
    total_score = max_score * 10 + sum_score; // 主方向权重更高

    // 位置奖励：越靠近中心分数越高
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // 曼哈顿距离
    int position_bonus = 50 * (BOARD_SIZE - distance);    // 距离中心越近奖励越高

    board[x][y] = original;              // 还原棋盘状态
    return total_score + position_bonus; // 返回总评估分
}

/**
 * @brief 使用带α-β剪枝的深度优先搜索（Minimax算法）来寻找最佳落子点。
 *        该函数递归地探索未来的几步棋，评估不同选择的优劣，并选择最优策略。
 * @param x 上一步落子的行坐标。
 * @param y 上一步落子的列坐标。
 * @param player 当前轮到的玩家 (PLAYER 或 AI)。
 * @param depth 剩余的搜索深度。深度越大，AI看得越远，但计算量也越大。
 * @param alpha α值，极大化玩家（AI）当前能确保的最好结果（下界）。
 * @param beta  β值，极小化玩家（对手）当前能确保的最好结果（上界）。
 * @param is_maximizing 布尔值，true表示当前是极大化玩家（AI）的回合，false表示是极小化玩家（对手）的回合。
 * @return int 返回在当前分支下的最佳评估分数。
 * @note 算法核心思想:
 *  1. **递归终止条件**: 当游戏出现胜负、平局，或达到预设的搜索深度时，停止递归，并返回当前局面的静态评估分数。
 *  2. **极大化玩家 (AI)**: 尝试所有可能的落子，并选择能使其得分最大化的那一步。此过程中，会不断更新alpha值。
 *  3. **极小化玩家 (对手)**: 模拟对手的走法，并假设对手会选择能使AI得分最小化的那一步。此过程中，会不断更新beta值。
 *  4. **α-β剪枝**: 这是对Minimax算法的关键优化。
 *     - **α剪枝**: 在极小化玩家的回合中，如果发现一个走法得到的分数比alpha还低，那么这个分支可以被剪掉，
 *       因为极大化玩家绝不会选择进入这个分支（他有更好的选择）。(if beta <= alpha)
 *     - **β剪枝**: 在极大化玩家的回ah合中，如果发现一个走法得到的分数比beta还高，那么这个分支也可以被剪掉，
 *       因为极小化玩家绝不会让游戏进入这个分支（他有更好的选择来阻止）。(if beta <= alpha)
 *     通过剪枝，可以避免对大量无效分支的搜索，极大地提高了AI的决策效率。
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing)
{
    // 检查当前落子是否获胜
    if (check_win(x, y, player))
    {
        return (player == AI) ? 1000000 + depth : -1000000 - depth;
    }

    // 达到搜索深度或平局
    if (depth == 0 || step_count >= BOARD_SIZE * BOARD_SIZE)
    {
        return evaluate_pos(x, y, AI) - evaluate_pos(x, y, PLAYER);
    }

    int best_score = is_maximizing ? -1000000 : 1000000;

    // 遍历所有可能落子位置
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 模拟当前玩家落子
            board[i][j] = player;
            step_count++;

            // 递归搜索(切换玩家和搜索深度)
            int current_score = dfs(i, j, (player == AI) ? PLAYER : AI, depth - 1, alpha, beta, !is_maximizing);

            // 撤销落子
            board[i][j] = EMPTY;
            step_count--;

            // 极大值玩家(AI)逻辑
            if (is_maximizing)
            {
                best_score = (current_score > best_score) ? current_score : best_score;
                alpha = (best_score > alpha) ? best_score : alpha;
                // α剪枝
                if (beta <= alpha)
                {
                    break;
                }
            }
            // 极小值玩家(人类)逻辑
            else
            {
                best_score = (current_score < best_score) ? current_score : best_score;
                beta = (best_score < beta) ? best_score : beta;
                // β剪枝
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        if ((is_maximizing && best_score >= beta) || (!is_maximizing && best_score <= alpha))
        {
            break; // 提前退出外层循环
        }
    }

    return best_score;
}

/**
 * @brief AI决策主函数，使用评估函数和搜索算法选择最佳落子位置
 * @note 采用两阶段决策逻辑：
 * 1. 防御阶段：检查并阻止玩家即将获胜的位置（活四、冲四、活三）
 * 2. 进攻阶段：若无紧急防御需求，使用DFS评估选择最佳进攻位置
 * @note 实现细节：
 * - 优先处理玩家活四、冲四等危险局面
 * - 步数>10时缩小搜索范围到已有棋子附近2格
 * - 使用中心位置优先策略
 */
void ai_move(int depth)
{
    // 1. 首先检查是否需要阻止玩家的四子连棋或三子活棋
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 模拟玩家在此位置落子
            board[i][j] = PLAYER;
            bool need_block = false;

            // 检查四个方向
            for (int k = 0; k < 4; k++)
            {
                DirInfo info = count_specific_direction(i, j, direction[k][0], direction[k][1], PLAYER);

                // 如果玩家能形成四子连棋且至少一端开放
                if (info.continuous_chess >= 4 && (info.check_start || info.check_end))
                {
                    need_block = true;
                    break;
                }

                // 如果玩家能形成三子活棋且两端开放
                if (info.continuous_chess == 3 && info.check_start && info.check_end)
                {
                    need_block = true;
                    break;
                }
            }

            board[i][j] = EMPTY; // 恢复棋盘

            if (need_block)
            {
                // 必须在此位置落子阻止
                board[i][j] = AI;
                steps[step_count++] = (Step){AI, i, j};
                printf("AI落子(%d, %d)\n", i + 1, j + 1);
                return;
            }
        }
    }

    // 2. 如果没有需要立即阻止的情况，则正常评估
    int best_score = -1000000;
    int best_x = -1, best_y = -1;

    // 遍历棋盘所有空位
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 只考虑已有棋子附近(2格范围内)
            bool has_nearby_stone = false;
            for (int di = -2; di <= 2; di++)
            {
                for (int dj = -2; dj <= 2; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < BOARD_SIZE &&
                        nj >= 0 && nj < BOARD_SIZE)
                    {
                        if (board[ni][nj] != EMPTY)
                        {
                            has_nearby_stone = true;
                            break;
                        }
                    }
                }
                if (has_nearby_stone)
                    break;
            }
            if (!has_nearby_stone && step_count > 10)
                continue;

            // 模拟AI落子
            board[i][j] = AI;
            int current_score = dfs(i, j, PLAYER, depth, -1000000, 1000000, false);
            board[i][j] = EMPTY;

            // 更新最佳位置
            if (current_score > best_score)
            {
                best_score = current_score;
                best_x = i;
                best_y = j;
            }
        }
    }

    // 执行最佳落子
    if (best_x != -1 && best_y != -1)
    {
        board[best_x][best_y] = AI;
        steps[step_count++] = (Step){AI, best_x, best_y};
        printf("AI落子(%d, %d)\n", best_x + 1, best_y + 1);
    }
}