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
 * @brief �������(x, y)λ���Ƿ�Ϊ��
 * @param x ������(0-base)
 * @param y ������(0-base)
 * @return true-��, false-�ǿ�
 */
bool have_space(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == EMPTY;
}

// ��������
/**
 * @brief ����Ƿ�Ϊ����
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
            return true; // ��������
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
        return true; // ���������Ľ���
    }

    return false;
}

/**
 * @brief ִ��������Ӳ���
 * @param x ������(0-base)
 * @param y ������(0-base)
 * @return true ���ӳɹ�
 * @return false ����ʧ��(λ����Ч)
 */
bool player_move(int x, int y, int player)
{
    // λ����Ч�򷵻�false
    if (!have_space(x, y))
        return false;

    if (is_forbidden_move(x, y, player))
    {
        printf("���֣���ѡ������λ�á�\n");
        return false;
    }

    // ��������״̬
    board[x][y] = player;
    // ��¼���Ӳ��裺��ұ�ʶ������
    steps[step_count++] = (Step){player, x, y};
    return true;
}

/**
 * @brief �����ض�����������ͬɫ��������
 * @param x ��ʼ������
 * @param y ��ʼ������
 * @param dx �з�������(-1,0,1)
 * @param dy �з�������(-1,0,1)
 * @param player ��ұ�ʶ(PLAYER/AI)
 * @return DirInfo ���������������ͷ��򿪷�״̬�Ľṹ��
 * @note ���������������ͳ���������������ж϶˵��Ƿ񿪷�
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player)
{
    DirInfo info;
    info.continuous_chess = 1; // ��ʼλ���Ѿ���һ������
    info.check_start = false;  // ��㷽���Ƿ񿪷�
    info.check_end = false;    // �յ㷽���Ƿ񿪷�

    // ���������dx, dy��
    int nx = x + dx, ny = y + dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // �������Ӽ�������
        nx += dx;                // �ص�ǰ����ǰ��
        ny += dy;
    }
    // �ж�������˵��Ƿ񿪷ţ�������λ��
    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE)
    {
        if (board[nx][ny] == EMPTY)
        {
            info.check_end = true;
        }
    }

    // ��鷴����-dx, -dy��
    nx = x - dx, ny = y - dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // �������Ӽ�������
        nx -= dx;                // ���෴����ǰ��
        ny -= dy;
    }
    // �жϷ�����˵��Ƿ񿪷ţ�������λ��
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
    // ����ĸ������Ƿ����������
    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);
        if (info.continuous_chess >= 5) // ��������>=5����ʤ
        {
            return true;
        }
    }
    return false; // �ĸ�����û��������
}

/**
 * @brief ���幦��ʵ��
 *
 * @param steps_to_undo Ҫ����Ĳ���
 * @return true ����ɹ�
 * @return false ����ʧ��(��������)
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
 * @brief �����������������еı���
 * @param player Ҫ���������(PLAYER/AI)
 * @return int �ܷ�(�ѿ��Ƿ����ظ�����)
 * @note �Ľ�������ֱ�׼:
 * - ����:5000 (���Ȩ�أ���ǿ����ʤ)
 * - ����:2000 ����:1000 ����:300 (���Ȩ�أ�ǿ��������)
 * - ����:500 ����:200 ����:80 (���Ȩ�أ�ǿ��ս�Լ�ֵ)
 * - ���:100 �߶�:40 ����:15 (�ʵ����Ȩ��)
 * - ���ŵ���:15 �뿪�ŵ���:8 ��յ���:2 (�ʵ����Ȩ��)
 * @note ʵ��ϸ��:
 * 1. ������������λ��
 * 2. ��ÿ�����Ӽ���ĸ�����
 * 3. ͳ�������������������
 * 4. ���շ�������4(���������ظ�����Ӱ��)
 */
int calculate_step_score(int x, int y, int player)
{
    int step_score = 0;
    // ����ĸ�����
    for (int k = 0; k < 4; k++)
    {
        DirInfo info = count_specific_direction(x, y, direction[k][0], direction[k][1], player);
        // ��������������
        switch (info.continuous_chess)
        {
        case 5:
            step_score += SCORE_FIVE;
            break; // ����
        case 4:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_FOUR; // ����
            else if (info.check_start || info.check_end)
                step_score += SCORE_RUSH_FOUR; // ����
            else
                step_score += SCORE_DEAD_FOUR; // ����
            break;
        case 3:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_THREE; // ����
            else if (info.check_start || info.check_end)
                step_score += SCORE_SLEEP_THREE; // ����
            else
                step_score += SCORE_DEAD_THREE; // ����
            break;
        case 2:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_TWO; // ���
            else if (info.check_start || info.check_end)
                step_score += SCORE_SLEEP_TWO; // �߶�
            else
                step_score += SCORE_DEAD_TWO; // ����
            break;
        case 1:
            if (info.check_start && info.check_end)
                step_score += SCORE_LIVE_ONE; // ���ŵ���
            else if (info.check_start || info.check_end)
                step_score += SCORE_HALF_ONE; // �뿪�ŵ���
            else
                step_score += SCORE_DEAD_ONE; // ��յ���
            break;
        }
    }
    
    // λ�ý�����Խ�������ķ���Խ��
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // �����پ���
    int position_bonus = POSITION_BONUS_FACTOR * (BOARD_SIZE - distance); // ��������Խ������Խ��
    
    return step_score + position_bonus;
}