#ifndef AI_H
#define AI_H

#include "gobang.h"

/**
 * @brief ����һ������λ�õ��ۺϵ÷֣���Ͻ����ͷ��أ�
 *
 * @param x ������
 * @param y ������
 * @return int �ۺϵ÷�
 */
int evaluate_move(int x, int y);

/**
 * @brief ����ָ��λ�õļ�ֵ
 *
 * @param x λ��x����
 * @param y λ��y����
 * @param player ��ұ�ʶ(PLAYER/AI)
 * @return int λ�ü�ֵ
 */
int evaluate_pos(int x, int y, int player);

/**
 * @brief �������̼�ֵ
 *
 * @param player ��ұ�ʶ(PLAYER/AI)
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing);

/**
 * @brief AI����
 *
 * @param depth
 */
void ai_move(int depth);

#endif // AI_H