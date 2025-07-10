#include "gobang.h"
#include "ai.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief 评估一个落子位置的综合得分（结合进攻和防守）
 * @param x 行坐标
 * @param y 列坐标
 * @return int 综合得分
 */
int evaluate_move(int x, int y)
{
    // 进攻得分：评估AI在此处落子的分数
    int attack_score = evaluate_pos(x, y, AI);

    // 防守得分：评估玩家在此处落子的分数，作为防守的依据
    int defense_score = evaluate_pos(x, y, PLAYER);

    // 综合得分，防守权重由 defense_coefficient 控制
    return attack_score + (int)(defense_score * defense_coefficient);
}

/**
 * @brief 评估特定位置对当前玩家的战略价值
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return int 综合评估分数(越高表示位置越好)
 * @note 评分标准:
 * - 活四:100000 冲四:10000 死四:500
 * - 活三:5000 眠三:1000 死三:50
 * - 活二:500 眠二:100 死二:10
 * - 单子:50(开放)/10(半开放)/1(封闭)
 * - 中心位置有额外加成
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
            return SEARCH_WIN_BONUS; // 返回最大分
        }

        // 根据连续棋子数评分
        switch (info.continuous_chess)
        {
        case 4:                                     // 四连珠
            if (info.check_start && info.check_end) // 活四(两端开放)
                line_scores[i] = AI_SCORE_LIVE_FOUR;
            else if (info.check_start || info.check_end) // 冲四(一端开放)
                line_scores[i] = AI_SCORE_RUSH_FOUR;
            else // 死四(两端封闭)
                line_scores[i] = AI_SCORE_DEAD_FOUR;
            break;

        case 3:                                     // 三连珠
            if (info.check_start && info.check_end) // 活三
                line_scores[i] = AI_SCORE_LIVE_THREE;
            else if (info.check_start || info.check_end) // 眠三
                line_scores[i] = AI_SCORE_SLEEP_THREE;
            else // 死三
                line_scores[i] = AI_SCORE_DEAD_THREE;
            break;

        case 2:                                     // 二连珠
            if (info.check_start && info.check_end) // 活二
                line_scores[i] = AI_SCORE_LIVE_TWO;
            else if (info.check_start || info.check_end) // 眠二
                line_scores[i] = AI_SCORE_SLEEP_TWO;
            else // 死二
                line_scores[i] = AI_SCORE_DEAD_TWO;
            break;

        case 1:                                     // 单子
            if (info.check_start && info.check_end) // 开放位置
                line_scores[i] = AI_SCORE_LIVE_ONE;
            else if (info.check_start || info.check_end) // 半开放位置
                line_scores[i] = AI_SCORE_HALF_ONE;
            else // 封闭位置
                line_scores[i] = AI_SCORE_DEAD_ONE;
            break;
        }
    }

    // 计算总分（最高方向分+其他方向分加权）
    int max_score = 0;
    int sum_score = 0;
    for (int i = 0; i < 4; i++)
    {
        if (line_scores[i] > max_score)
        {
            max_score = line_scores[i];
        }
        sum_score += line_scores[i];
    }
    total_score = max_score * 10 + sum_score; // 主方向权重更高

    // 位置奖励：越靠近中心分数越高
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // 曼哈顿距离
    int position_bonus = AI_POSITION_BONUS_FACTOR * (BOARD_SIZE - distance);    // 距离中心越近奖励越高

    board[x][y] = original;              // 还原棋盘状态
    return total_score + position_bonus; // 返回总评估分
}

/**
 * @brief 带α-β剪枝的深度优先搜索(极小极大算法实现)
 * @param x 当前行坐标
 * @param y 当前列坐标
 * @param player 当前玩家
 * @param depth 剩余搜索深度
 * @param alpha α值(当前最大值)
 * @param beta β值(当前最小值)
 * @param is_maximizing 是否为极大化玩家(AI)
 * @return int 最佳评估分数
 * @note 算法流程:
 * 1. 检查是否获胜或达到搜索深度
 * 2. 遍历所有可能落子位置
 * 3. 递归评估每个位置的分数
 * 4. 根据is_maximizing选择最大/最小值
 * 5. 使用α-β剪枝优化搜索过程
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing)
{
    // 检查当前落子是否获胜
    if (check_win(x, y, player))
    {
        return (player == AI) ? SEARCH_WIN_BONUS + depth : -SEARCH_WIN_BONUS - depth;
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
            {
                continue;
            }

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
 * 2. 进攻阶段：若无紧急防御需求，使用评估函数选择最佳进攻位置
 * @note 实现细节：
 * - 优先处理玩家活四、冲四等危险局面
 * - 步数>AI_SEARCH_RANGE_THRESHOLD时缩小搜索范围到已有棋子附近AI_NEARBY_RANGE格
 * - 使用中心位置优先策略
 */
void ai_move(int depth)
{
    // 如果是第一步，直接下在中心位置附近
    if (step_count == 0)
    {
        int center = BOARD_SIZE / 2;
        board[center][center] = AI;
        steps[step_count++] = (Step){AI, center, center};
        printf("AI落子(%d, %d)\n", center + 1, center + 1);
        return;
    }

    // 1. 首先检查是否需要阻止玩家的四子连棋或三子活棋
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

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
    int best_score = -SEARCH_WIN_BONUS;
    int best_x = -1, best_y = -1;

    // 遍历棋盘所有空位
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

            // 只考虑已有棋子附近(AI_NEARBY_RANGE格范围内)
            bool has_nearby_stone = false;
            for (int di = -AI_NEARBY_RANGE; di <= AI_NEARBY_RANGE; di++)
            {
                for (int dj = -AI_NEARBY_RANGE; dj <= AI_NEARBY_RANGE; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE)
                    {
                        if (board[ni][nj] != EMPTY)
                        {
                            has_nearby_stone = true;
                            break;
                        }
                    }
                }
                if (has_nearby_stone)
                {
                    break;
                }
            }
            if (!has_nearby_stone && step_count > AI_SEARCH_RANGE_THRESHOLD)
            {
                continue;
            }

            // 使用评估函数获取综合得分
            int current_score = evaluate_move(i, j);

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