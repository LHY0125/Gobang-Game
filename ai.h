/**
 * @file ai.h
 * @note 本文件定义了AI模块的函数和变量
 * @note 包括：
 * 1. 评估一个落子位置的综合得分（结合进攻和防守）
 * 2. 评估指定位置的价值
 * 3. 评估棋盘价值
 */
#ifndef AI_H
#define AI_H

#include "gobang.h"
#include "type.h"

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

// ==================== AI增强：新增函数声明 ====================

/**
 * @brief 生成候选移动并按评估分数排序
 * @param moves 存储候选移动的数组
 * @param player 当前玩家
 * @return 候选移动数量
 */
int generate_candidate_moves(ScoredMove *moves, int player);

/**
 * @brief 检查位置是否在已有棋子附近
 * @param x, y 要检查的位置
 * @return 如果附近有棋子返回true
 */
bool is_near_stones(int x, int y);

/**
 * @brief 检测在指定位置落子的威胁等级
 * @param x, y 落子位置
 * @param player 落子玩家
 * @return 威胁等级
 */
ThreatLevel detect_threat(int x, int y, int player);

/**
 * @brief 计算指定方向的威胁数量
 * @param x, y 起始位置
 * @param dx, dy 方向向量
 * @param player 玩家
 * @return 威胁数量
 */
int count_threats_in_direction(int x, int y, int dx, int dy, int player);

#endif // AI_H