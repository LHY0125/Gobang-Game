#ifndef INIT_BOARD_H
#define INIT_BOARD_H

#include "gobang.h"

// --- 游戏初始化 ---
/**
 * @brief 初始化棋盘，将所有位置设置为空(EMPTY)
 */
void empty_board();

/**
 * @brief 将当前棋盘状态打印到控制台
 */
void print_board();

/**
 * @brief 设置当前游戏的棋盘大小
 */
void setup_board_size();

/**
 * @brief 设置游戏选项，如是否启用禁手、计时器等
 */
void setup_game_options();

/**
 * @brief 决定先手玩家
 * @param player1 玩家1的标识
 * @param player2 玩家2的标识
 * @return 返回先手玩家的标识
 */
int determine_first_player(int player1, int player2);

#endif // INIT_H