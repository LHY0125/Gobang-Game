#ifndef AI_H
#define AI_H

#include "gobang.h"

/**
 * @brief 评估特定位置对当前玩家的价值
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return int 位置评估分数(越高越好)
 */
int evaluate_pos(int x, int y, int player);

/**
 * @brief 带α-β剪枝的深度优先搜索(极小极大算法)
 * @param x 当前行坐标
 * @param y 当前列坐标
 * @param player 当前玩家
 * @param depth 搜索深度
 * @param alpha α值(当前最大值)
 * @param beta β值(当前最小值)
 * @param is_maximizing 是否为极大化玩家
 * @return int 最佳评估分数
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing);

/**
 * @brief AI落子决策函数
 * 使用评估函数和搜索算法选择最佳落子位置
 */
void ai_move(int depth);

#endif // AI_H