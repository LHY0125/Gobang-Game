/**
 * @file ai.c
 * @note 本文件定义了AI模块的函数和变量
 * @note 包括：
 * 1. 评估一个落子位置的综合得分（结合进攻和防守）
 * 2. 评估指定位置的价值
 * 3. 评估棋盘价值
 */

#include "gobang.h"
#include "ai.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ==================== 辅助函数声明 ====================
static int compare_moves(const void *a, const void *b);

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
            board[x][y] = original;  // 还原棋盘
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
    int distance = abs(x - center_x) + abs(y - center_y);                    // 曼哈顿距离
    int position_bonus = AI_POSITION_BONUS_FACTOR * (BOARD_SIZE - distance); // 距离中心越近奖励越高

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

    // 使用移动排序优化搜索效率
    ScoredMove candidate_moves[BOARD_SIZE * BOARD_SIZE];
    int move_count = generate_candidate_moves(candidate_moves, player);

    // 限制搜索的候选移动数量以提高性能
    int max_candidates = (depth >= 3) ? 15 : 25; // 深度越大，候选移动越少
    if (move_count > max_candidates)
    {
        move_count = max_candidates;
    }

    // 遍历排序后的候选移动
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

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

    // 1. 使用增强的威胁检测系统
    ScoredMove candidate_moves[BOARD_SIZE * BOARD_SIZE];
    int move_count = generate_candidate_moves(candidate_moves, AI);

    // 首先检查是否有直接获胜的机会
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

        ThreatLevel ai_threat = detect_threat(i, j, AI);
        if (ai_threat == THREAT_WIN)
        {
            // 直接获胜
            board[i][j] = AI;
            steps[step_count++] = (Step){AI, i, j};
            printf("AI落子(%d, %d) - 获胜!\n", i + 1, j + 1);
            return;
        }
    }

    // 检查是否需要阻止玩家的威胁
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

        ThreatLevel player_threat = detect_threat(i, j, PLAYER);
        if (player_threat >= THREAT_FOUR)
        {
            // 必须阻止玩家的四子威胁
            board[i][j] = AI;
            steps[step_count++] = (Step){AI, i, j};
            printf("AI落子(%d, %d) - 防守!\n", i + 1, j + 1);
            return;
        }
    }

    // 检查是否需要阻止玩家的活三威胁
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

        ThreatLevel player_threat = detect_threat(i, j, PLAYER);
        if (player_threat == THREAT_THREE)
        {
            // 阻止玩家的活三
            board[i][j] = AI;
            steps[step_count++] = (Step){AI, i, j};
            printf("AI落子(%d, %d) - 阻止活三!\n", i + 1, j + 1);
            return;
        }
    }

    // 2. 寻找最佳进攻位置
    // 优先考虑能形成威胁的位置
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

        ThreatLevel ai_threat = detect_threat(i, j, AI);
        if (ai_threat >= THREAT_FOUR)
        {
            // 形成四子威胁
            board[i][j] = AI;
            steps[step_count++] = (Step){AI, i, j};
            printf("AI落子(%d, %d) - 形成威胁!\n", i + 1, j + 1);
            return;
        }
    }

    // 寻找能形成活三的位置
    for (int idx = 0; idx < move_count; idx++)
    {
        int i = candidate_moves[idx].x;
        int j = candidate_moves[idx].y;

        ThreatLevel ai_threat = detect_threat(i, j, AI);
        if (ai_threat == THREAT_THREE)
        {
            // 形成活三
            board[i][j] = AI;
            steps[step_count++] = (Step){AI, i, j};
            printf("AI落子(%d, %d) - 形成活三!\n", i + 1, j + 1);
            return;
        }
    }

    // 3. 如果没有明显的威胁机会，选择评分最高的位置
    if (move_count > 0)
    {
        // candidate_moves已经按分数排序，直接选择第一个
        int best_x = candidate_moves[0].x;
        int best_y = candidate_moves[0].y;

        board[best_x][best_y] = AI;
        steps[step_count++] = (Step){AI, best_x, best_y};
        printf("AI落子(%d, %d) - 最佳位置!\n", best_x + 1, best_y + 1);
    }
    else
    {
        // 备用方案：如果没有候选移动，随机选择一个位置
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                if (board[i][j] == EMPTY)
                {
                    board[i][j] = AI;
                    steps[step_count++] = (Step){AI, i, j};
                    printf("AI落子(%d, %d) - 备用位置!\n", i + 1, j + 1);
                    return;
                }
            }
        }
    }
}

// ==================== AI增强：辅助函数实现 ====================

/**
 * @brief 比较函数，用于移动排序（按分数降序）
 */
static int compare_moves(const void *a, const void *b)
{
    const ScoredMove *move_a = (const ScoredMove *)a;
    const ScoredMove *move_b = (const ScoredMove *)b;
    return move_b->score - move_a->score; // 降序排列
}

/**
 * @brief 生成候选移动并按评估分数排序
 * @param moves 存储候选移动的数组
 * @param player 当前玩家
 * @return 候选移动数量
 */
int generate_candidate_moves(ScoredMove *moves, int player)
{
    int count = 0;

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

            // 只考虑有意义的位置（附近有棋子）
            if (step_count > AI_SEARCH_RANGE_THRESHOLD && !is_near_stones(i, j))
            {
                continue;
            }

            // 计算该位置的评估分数
            moves[count].x = i;
            moves[count].y = j;

            // 结合威胁检测和位置评估
            ThreatLevel threat = detect_threat(i, j, player);
            int base_score = evaluate_move(i, j);

            // 根据威胁等级调整分数
            switch (threat)
            {
            case THREAT_WIN:
                moves[count].score = base_score + 10000;
                break;
            case THREAT_FOUR:
                moves[count].score = base_score + 5000;
                break;
            case THREAT_THREE:
                moves[count].score = base_score + 2000;
                break;
            case THREAT_DOUBLE:
                moves[count].score = base_score + 1000;
                break;
            case THREAT_POTENTIAL:
                moves[count].score = base_score + 500;
                break;
            default:
                moves[count].score = base_score;
                break;
            }

            count++;
        }
    }

    // 按分数降序排序
    qsort(moves, count, sizeof(ScoredMove), compare_moves);

    return count;
}

/**
 * @brief 检查位置是否在已有棋子附近
 * @param x, y 要检查的位置
 * @return 如果附近有棋子返回true
 */
bool is_near_stones(int x, int y)
{
    for (int di = -AI_NEARBY_RANGE; di <= AI_NEARBY_RANGE; di++)
    {
        for (int dj = -AI_NEARBY_RANGE; dj <= AI_NEARBY_RANGE; dj++)
        {
            int ni = x + di;
            int nj = y + dj;
            if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE)
            {
                if (board[ni][nj] != EMPTY)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * @brief 检测在指定位置落子的威胁等级
 * @param x, y 落子位置
 * @param player 落子玩家
 * @return 威胁等级
 */
ThreatLevel detect_threat(int x, int y, int player)
{
    // 模拟落子
    board[x][y] = player;

    ThreatLevel max_threat = THREAT_NONE;
    int threat_count = 0;

    // 检查四个方向
    for (int k = 0; k < 4; k++)
    {
        DirInfo info = count_specific_direction(x, y, direction[k][0], direction[k][1], player);
        ThreatLevel current_threat = THREAT_NONE;

        // 检查是否形成五子连珠（获胜）
        if (info.continuous_chess >= 5)
        {
            current_threat = THREAT_WIN;
        }
        // 检查是否形成活四或冲四
        else if (info.continuous_chess == 4)
        {
            if (info.check_start && info.check_end)
            {
                current_threat = THREAT_FOUR; // 活四
            }
            else if (info.check_start || info.check_end)
            {
                current_threat = THREAT_FOUR; // 冲四
            }
        }
        // 检查是否形成活三
        else if (info.continuous_chess == 3 && info.check_start && info.check_end)
        {
            current_threat = THREAT_THREE;
        }
        // 检查潜在威胁
        else if (info.continuous_chess >= 2)
        {
            current_threat = THREAT_POTENTIAL;
        }

        if (current_threat > max_threat)
        {
            max_threat = current_threat;
        }

        if (current_threat >= THREAT_THREE)
        {
            threat_count++;
        }
    }

    // 恢复棋盘
    board[x][y] = EMPTY;

    // 如果有多个威胁，提升威胁等级
    if (threat_count >= 2 && max_threat >= THREAT_THREE)
    {
        max_threat = THREAT_DOUBLE;
    }

    return max_threat;
}

/**
 * @brief 计算指定方向的威胁数量
 * @param x, y 起始位置
 * @param dx, dy 方向向量
 * @param player 玩家
 * @return 威胁数量
 */
int count_threats_in_direction(int x, int y, int dx, int dy, int player)
{
    int threats = 0;

    // 向前搜索
    for (int i = 1; i < 5; i++)
    {
        int nx = x + i * dx;
        int ny = y + i * dy;

        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
        {
            break;
        }

        if (board[nx][ny] == player)
        {
            threats++;
        }
        else if (board[nx][ny] != EMPTY)
        {
            break;
        }
    }

    // 向后搜索
    for (int i = 1; i < 5; i++)
    {
        int nx = x - i * dx;
        int ny = y - i * dy;

        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
        {
            break;
        }

        if (board[nx][ny] == player)
        {
            threats++;
        }
        else if (board[nx][ny] != EMPTY)
        {
            break;
        }
    }

    return threats;
}