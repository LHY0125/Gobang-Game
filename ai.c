#include "gobang.h"
#include "ai.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief ����һ������λ�õ��ۺϵ÷֣���Ͻ����ͷ��أ�
 * @param x ������
 * @param y ������
 * @return int �ۺϵ÷�
 */
int evaluate_move(int x, int y)
{
    // �����÷֣�����AI�ڴ˴����ӵķ���
    int attack_score = evaluate_pos(x, y, AI);

    // ���ص÷֣���������ڴ˴����ӵķ�������Ϊ���ص�����
    int defense_score = evaluate_pos(x, y, PLAYER);

    // �ۺϵ÷֣�����Ȩ���� defense_coefficient ����
    return attack_score + (int)(defense_score * defense_coefficient);
}

/**
 * @brief �����ض�λ�öԵ�ǰ��ҵ�ս�Լ�ֵ
 * @param x ������(0-base)
 * @param y ������(0-base)
 * @param player ��ұ�ʶ(PLAYER/AI)
 * @return int �ۺ���������(Խ�߱�ʾλ��Խ��)
 * @note ���ֱ�׼:
 * - ����:100000 ����:10000 ����:500
 * - ����:5000 ����:1000 ����:50
 * - ���:500 �߶�:100 ����:10
 * - ����:50(����)/10(�뿪��)/1(���)
 * - ����λ���ж���ӳ�
 */
int evaluate_pos(int x, int y, int player)
{
    // ����ԭʼֵ���ڻ�ԭ
    int original = board[x][y];
    // ģ���ڸ�λ������
    board[x][y] = player;

    int total_score = 0;      // �ܷ�
    int line_scores[4] = {0}; // �ĸ�����ĵ÷�

    // �����ĸ������������
    for (int i = 0; i < 4; i++)
    {
        int dx = direction[i][0], dy = direction[i][1];
        // ��ȡ��ǰ�����ϵ�������Ϣ
        DirInfo info = count_specific_direction(x, y, dx, dy, player);

        // ֱ���γ�������Ϊ��ʤ
        if (info.continuous_chess >= 5)
        {
            board[x][y] = original; // ��ԭ����
            return SEARCH_WIN_BONUS; // ��������
        }

        // ������������������
        switch (info.continuous_chess)
        {
        case 4:                                     // ������
            if (info.check_start && info.check_end) // ����(���˿���)
                line_scores[i] = AI_SCORE_LIVE_FOUR;
            else if (info.check_start || info.check_end) // ����(һ�˿���)
                line_scores[i] = AI_SCORE_RUSH_FOUR;
            else // ����(���˷��)
                line_scores[i] = AI_SCORE_DEAD_FOUR;
            break;

        case 3:                                     // ������
            if (info.check_start && info.check_end) // ����
                line_scores[i] = AI_SCORE_LIVE_THREE;
            else if (info.check_start || info.check_end) // ����
                line_scores[i] = AI_SCORE_SLEEP_THREE;
            else // ����
                line_scores[i] = AI_SCORE_DEAD_THREE;
            break;

        case 2:                                     // ������
            if (info.check_start && info.check_end) // ���
                line_scores[i] = AI_SCORE_LIVE_TWO;
            else if (info.check_start || info.check_end) // �߶�
                line_scores[i] = AI_SCORE_SLEEP_TWO;
            else // ����
                line_scores[i] = AI_SCORE_DEAD_TWO;
            break;

        case 1:                                     // ����
            if (info.check_start && info.check_end) // ����λ��
                line_scores[i] = AI_SCORE_LIVE_ONE;
            else if (info.check_start || info.check_end) // �뿪��λ��
                line_scores[i] = AI_SCORE_HALF_ONE;
            else // ���λ��
                line_scores[i] = AI_SCORE_DEAD_ONE;
            break;
        }
    }

    // �����ܷ֣���߷����+��������ּ�Ȩ��
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
    total_score = max_score * 10 + sum_score; // ������Ȩ�ظ���

    // λ�ý�����Խ�������ķ���Խ��
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // �����پ���
    int position_bonus = AI_POSITION_BONUS_FACTOR * (BOARD_SIZE - distance);    // ��������Խ������Խ��

    board[x][y] = original;              // ��ԭ����״̬
    return total_score + position_bonus; // ������������
}

/**
 * @brief ����-�¼�֦�������������(��С�����㷨ʵ��)
 * @param x ��ǰ������
 * @param y ��ǰ������
 * @param player ��ǰ���
 * @param depth ʣ���������
 * @param alpha ��ֵ(��ǰ���ֵ)
 * @param beta ��ֵ(��ǰ��Сֵ)
 * @param is_maximizing �Ƿ�Ϊ�������(AI)
 * @return int �����������
 * @note �㷨����:
 * 1. ����Ƿ��ʤ��ﵽ�������
 * 2. �������п�������λ��
 * 3. �ݹ�����ÿ��λ�õķ���
 * 4. ����is_maximizingѡ�����/��Сֵ
 * 5. ʹ�æ�-�¼�֦�Ż���������
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing)
{
    // ��鵱ǰ�����Ƿ��ʤ
    if (check_win(x, y, player))
    {
        return (player == AI) ? SEARCH_WIN_BONUS + depth : -SEARCH_WIN_BONUS - depth;
    }

    // �ﵽ������Ȼ�ƽ��
    if (depth == 0 || step_count >= BOARD_SIZE * BOARD_SIZE)
    {
        return evaluate_pos(x, y, AI) - evaluate_pos(x, y, PLAYER);
    }

    int best_score = is_maximizing ? -1000000 : 1000000;

    // �������п�������λ��
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

            // ģ�⵱ǰ�������
            board[i][j] = player;
            step_count++;

            // �ݹ�����(�л���Һ��������)
            int current_score = dfs(i, j, (player == AI) ? PLAYER : AI, depth - 1, alpha, beta, !is_maximizing);

            // ��������
            board[i][j] = EMPTY;
            step_count--;

            // ����ֵ���(AI)�߼�
            if (is_maximizing)
            {
                best_score = (current_score > best_score) ? current_score : best_score;
                alpha = (best_score > alpha) ? best_score : alpha;
                // ����֦
                if (beta <= alpha)
                {
                    break;
                }
            }
            // ��Сֵ���(����)�߼�
            else
            {
                best_score = (current_score < best_score) ? current_score : best_score;
                beta = (best_score < beta) ? best_score : beta;
                // �¼�֦
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        if ((is_maximizing && best_score >= beta) || (!is_maximizing && best_score <= alpha))
        {
            break; // ��ǰ�˳����ѭ��
        }
    }

    return best_score;
}

/**
 * @brief AI������������ʹ�����������������㷨ѡ���������λ��
 * @note �������׶ξ����߼���
 * 1. �����׶Σ���鲢��ֹ��Ҽ�����ʤ��λ�ã����ġ����ġ�������
 * 2. �����׶Σ����޽�����������ʹ����������ѡ����ѽ���λ��
 * @note ʵ��ϸ�ڣ�
 * - ���ȴ�����һ��ġ����ĵ�Σ�վ���
 * - ����>AI_SEARCH_RANGE_THRESHOLDʱ��С������Χ���������Ӹ���AI_NEARBY_RANGE��
 * - ʹ������λ�����Ȳ���
 */
void ai_move(int depth)
{
    // ����ǵ�һ����ֱ����������λ�ø���
    if (step_count == 0)
    {
        int center = BOARD_SIZE / 2;
        board[center][center] = AI;
        steps[step_count++] = (Step){AI, center, center};
        printf("AI����(%d, %d)\n", center + 1, center + 1);
        return;
    }

    // 1. ���ȼ���Ƿ���Ҫ��ֹ��ҵ�������������ӻ���
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

            // ģ������ڴ�λ������
            board[i][j] = PLAYER;
            bool need_block = false;

            // ����ĸ�����
            for (int k = 0; k < 4; k++)
            {
                DirInfo info = count_specific_direction(i, j, direction[k][0], direction[k][1], PLAYER);

                // ���������γ���������������һ�˿���
                if (info.continuous_chess >= 4 && (info.check_start || info.check_end))
                {
                    need_block = true;
                    break;
                }

                // ���������γ����ӻ��������˿���
                if (info.continuous_chess == 3 && info.check_start && info.check_end)
                {
                    need_block = true;
                    break;
                }
            }

            board[i][j] = EMPTY; // �ָ�����

            if (need_block)
            {
                // �����ڴ�λ��������ֹ
                board[i][j] = AI;
                steps[step_count++] = (Step){AI, i, j};
                printf("AI����(%d, %d)\n", i + 1, j + 1);
                return;
            }
        }
    }

    // 2. ���û����Ҫ������ֹ�����������������
    int best_score = -SEARCH_WIN_BONUS;
    int best_x = -1, best_y = -1;

    // �����������п�λ
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                continue;
            }

            // ֻ�����������Ӹ���(AI_NEARBY_RANGE��Χ��)
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

            // ʹ������������ȡ�ۺϵ÷�
            int current_score = evaluate_move(i, j);

            // �������λ��
            if (current_score > best_score)
            {
                best_score = current_score;
                best_x = i;
                best_y = j;
            }
        }
    }

    // ִ���������
    if (best_x != -1 && best_y != -1)
    {
        board[best_x][best_y] = AI;
        steps[step_count++] = (Step){AI, best_x, best_y};
        printf("AI����(%d, %d)\n", best_x + 1, best_y + 1);
    }
}