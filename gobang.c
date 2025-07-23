/**
 * @file gobang.c
 * @brief 五子棋游戏源文件
 * @note 本文件定义了五子棋游戏的主要数据结构、函数和全局变量。
 * 它包含了游戏棋盘的表示、玩家操作、规则检查以及AI决策等功能。
 */

#include "game_mode.h"
#include "init_board.h"
#include "gobang.h"
#include "ai.h"
#include "record.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

/**
 * @brief 检查棋盘(x, y)位置是否为空
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true-空, false-非空
 */
bool have_space(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == EMPTY;
}

// 函数定义
/**
 * @brief 检查是否为禁手
 *
 * @param x
 * @param y
 * @param player
 * @return true
 * @return false
 */
bool is_forbidden_move(int x, int y, int player)
{
    if (!use_forbidden_moves)
    {
        return false;
    }
    if (player != PLAYER && player != PLAYER1)
    {
        return false;
    }

    board[x][y] = player;

    int three_count = 0;
    int four_count = 0;

    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);

        if (info.continuous_chess > 5)
        {
            board[x][y] = EMPTY;
            return true; // 长连禁手
        }
        if (info.continuous_chess == 3 && info.check_start && info.check_end)
        {
            three_count++;
        }
        if (info.continuous_chess == 4 && (info.check_start || info.check_end))
        {
            four_count++;
        }
    }

    board[x][y] = EMPTY;

    if (three_count >= 2 || four_count >= 2)
    {
        return true; // 三三或四四禁手
    }

    return false;
}

/**
 * @brief 执行玩家落子操作
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 落子成功
 * @return false 落子失败(位置无效)
 */
bool player_move(int x, int y, int player)
{
    // 位置无效则返回false
    if (!have_space(x, y))
        return false;

    if (is_forbidden_move(x, y, player))
    {
        printf("禁手！请选择其他位置。\n");
        return false;
    }

    // 更新棋盘状态
    board[x][y] = player;
    // 记录落子步骤：玩家标识和坐标
    steps[step_count++] = (Step){player, x, y};
    return true;
}

/**
 * @brief 计算特定方向上连续同色棋子数量
 * @param x 起始行坐标
 * @param y 起始列坐标
 * @param dx 行方向增量(-1,0,1)
 * @param dy 列方向增量(-1,0,1)
 * @param player 玩家标识(PLAYER/AI)
 * @return DirInfo 包含连续棋子数和方向开放状态的结构体
 * @note 检查正反两个方向，统计连续棋子数并判断端点是否开放
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player)
{
    DirInfo info;
    info.continuous_chess = 1; // 起始位置已经有一个棋子
    info.check_start = false;  // 起点方向是否开放
    info.check_end = false;    // 终点方向是否开放

    // 检查正方向（dx, dy）
    int nx = x + dx, ny = y + dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // 连续棋子计数增加
        nx += dx;                // 沿当前方向前进
        ny += dy;
    }
    // 判断正方向端点是否开放（遇到空位）
    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE)
    {
        if (board[nx][ny] == EMPTY)
        {
            info.check_end = true;
        }
    }

    // 检查反方向（-dx, -dy）
    nx = x - dx, ny = y - dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // 连续棋子计数增加
        nx -= dx;                // 沿相反方向前进
        ny -= dy;
    }
    // 判断反方向端点是否开放（遇到空位）
    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE)
    {
        if (board[nx][ny] == EMPTY)
        {
            info.check_start = true;
        }
    }

    return info;
}

bool check_win(int x, int y, int player)
{
    // 检查四个方向是否存在五连珠
    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);
        if (info.continuous_chess >= 5) // 连续棋子>=5即获胜
        {
            return true;
        }
    }
    return false; // 四个方向都没有五连珠
}

/**
 * @brief 悔棋功能实现
 *
 * @param steps_to_undo 要悔棋的步数
 * @return true 悔棋成功
 * @return false 悔棋失败(步数不足)
 */
bool return_move(int steps_to_undo)
{
    if (step_count < steps_to_undo)
    {
        return false;
    }

    for (int i = 0; i < steps_to_undo; i++)
    {
        if (step_count > 0)
        {
            step_count--;
            board[steps[step_count].x][steps[step_count].y] = EMPTY;
        }
    }

    return true;
}

/**
 * @brief 评估玩家在整盘棋局中的表现
 * @param player 要评估的玩家(PLAYER/AI)
 * @return int 总分(已考虑方向重复计算)
 * @note 改进后的评分标准:
 * - 五连:5000 (提高权重，更强调获胜)
 * - 活四:2000 冲四:1000 死四:300 (提高权重，强调进攻性)
 * - 活三:500 眠三:200 死三:80 (提高权重，强调战略价值)
 * - 活二:100 眠二:40 死二:15 (适当提高权重)
 * - 开放单子:15 半开放单子:8 封闭单子:2 (适当提高权重)
 * @note 实现细节:
 * 1. 遍历棋盘所有位置
 * 2. 对每个棋子检查四个方向
 * 3. 统计所有连子情况并评分
 * 4. 最终分数除以4(消除方向重复计算影响)
 */
int calculate_step_score(int x, int y, int player)
{
    int step_score = 0;
    // 检查四个方向
    for (int k = 0; k < 4; k++)
    {
        DirInfo info = count_specific_direction(x, y, direction[k][0], direction[k][1], player);
        // 根据连子数评分
        switch (info.continuous_chess)
        {
        case 5:
            step_score += SCORE_FIVE;
            break; // 五连
        case 4:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_FOUR; // 活四
            else if (info.check_start || info.check_end)
                step_score += SCORE_RUSH_FOUR; // 冲四
            else
                step_score += SCORE_DEAD_FOUR; // 死四
            break;
        case 3:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_THREE; // 活三
            else if (info.check_start || info.check_end)
                step_score += SCORE_SLEEP_THREE; // 眠三
            else
                step_score += SCORE_DEAD_THREE; // 死三
            break;
        case 2:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_TWO; // 活二
            else if (info.check_start || info.check_end)
                step_score += SCORE_SLEEP_TWO; // 眠二
            else
                step_score += SCORE_DEAD_TWO; // 死二
            break;
        case 1:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_ONE; // 开放单子
            else if (info.check_start || info.check_end)
                step_score += SCORE_HALF_ONE; // 半开放单子
            else
                step_score += SCORE_DEAD_ONE; // 封闭单子
            break;
        }
    }
    
    // 位置奖励：越靠近中心分数越高
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // 曼哈顿距离
    int position_bonus = POSITION_BONUS_FACTOR * (BOARD_SIZE - distance); // 距离中心越近奖励越高
    
    return step_score + position_bonus;
}