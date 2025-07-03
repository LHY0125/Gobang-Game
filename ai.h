#ifndef AI_H
#define AI_H

#include "gobang.h"

// 防守系数
extern double defense_coefficient;

/**
 * @brief 评估一个落子位置的综合得分（结合进攻和防守）
 *
 * @param x 行坐标
 * @param y 列坐标
 * @return int 综合得分
 */
int evaluate_move(int x, int y);

/**
 * @brief 评估指定位置的价值
 *
 * @param x 位置x坐标
 * @param y 位置y坐标
 * @param player 玩家标识(PLAYER/AI)
 * @return int 位置价值
 */
int evaluate_pos(int x, int y, int player);

/**
 * @brief 评估棋盘价值
 *
 * @param player 玩家标识(PLAYER/AI)
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing);

/**
 * @brief AI下棋
 *
 * @param depth
 */
void ai_move(int depth);

#endif // AI_H