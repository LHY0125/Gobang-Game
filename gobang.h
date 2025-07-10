#ifndef GO_BANG_H
#define GO_BANG_H

#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// 数据结构

/**
 * @brief 记录一步棋的详细信息
 */
typedef struct
{
    int player; // 执行该步的玩家标识
    int x;      // 落子的行坐标 (0-based)
    int y;      // 落子的列坐标 (0-based)
} Step;

/**
 * @brief 存储在特定方向上棋子连续性的信息
 * @details 用于评估棋形，例如判断活三、冲四等关键形态。
 */
typedef struct
{
    int continuous_chess; // 连续同色棋子的数量
    bool check_start;     // 棋子序列的起始端是否为空位（即是否开放）
    bool check_end;       // 棋子序列的末尾端是否为空位（即是否开放）
} DirInfo;

// 函数原型

// --- 游戏核心逻辑 ---
/**
 * @brief 检查指定坐标是否为有效落子点（在棋盘内且为空）
 * @param x 待检查的行坐标 (0-based)
 * @param y 待检查的列坐标 (0-based)
 * @return 若位置有效且为空则返回true，否则返回false
 */
bool have_space(int x, int y);

/**
 * @brief 判断一个落子是否为禁手
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 如果是禁手则返回true，否则返回false
 */
bool is_forbidden_move(int x, int y, int player);


/**
 * @brief 执行一次玩家落子操作
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 若落子成功则返回true，否则（位置无效或被占用）返回false
 */
bool player_move(int x, int y, int player);

/**
 * @brief 计算在特定方向上的棋子连续信息
 * @param x 起始点的行坐标
 * @param y 起始点的列坐标
 * @param dx x方向的增量 (-1, 0, or 1)
 * @param dy y方向的增量 (-1, 0, or 1)
 * @param player 玩家标识
 * @return 返回一个包含连续棋子信息的 DirInfo 结构体
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player);

/**
 * @brief 检查在某点落子后，该玩家是否获胜
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 如果获胜则返回true，否则返回false
 */
bool check_win(int x, int y, int player);

/**
 * @brief 悔棋功能，撤销指定步数
 * @param steps_to_undo 要撤销的步数（每步包含双方各一次落子）
 * @return 若悔棋成功则返回true，否则返回false
 */
bool return_move(int steps_to_undo);

/**
 * @brief 计算并返回一步棋的得分
 * @param x 落子的行坐标
 * @param y 落子的列坐标
 * @param player 玩家标识
 * @return 该步棋的得分
 */
int calculate_step_score(int x, int y, int player);

#endif // GO_BANG_H