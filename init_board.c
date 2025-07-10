#include "init_board.h"
#include "gobang.h"
#include "game_mode.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief ��ʼ������Ϊȫ��״̬�����ò���������
 * ����������鲢������λ����ΪEMPTY��ͬʱ��step_count����Ϊ0
 */
void empty_board()
{
    // ��ʼ������״̬Ϊȫ��
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }
    step_count = 0; // ���ò���������
}

/**
 * @brief ��ӡ��ǰ����״̬
 * �Կɶ���ʽ������̣��������кź�����״̬
 * ���������ʾΪ'x'��AI������ʾΪ'��'����λ��ʾΪ'��'
 */
void print_board()
{
    // ��ӡ�кţ�1-BOARD_SIZE��ʾ��
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d", i + 1);
        if (i + 1 == 9) // �����к�9��10+�Ķ���
        {
            printf(" ");
        }
    }
    printf("\n");

    // ���д�ӡ��������
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d ", i + 1); // ��ӡ�кţ�1-BOARD_SIZE��
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == PLAYER)
            {
                printf("x "); // �������
            }
            else if (board[i][j] == AI)
            {
                printf("�� "); // AI����(ʹ�á���ʾ)
            }
            else
            {
                printf("�� "); // ��λ
            }
        }
        printf("\n"); // ÿ�н�������
    }
}

/**
 * @brief �������̴�С
 *
 * @param player1 ���1
 * @param player2 ���2
 */
void setup_board_size()
{
    printf("ͨ�����̴�С��Ϊ��������(13X13)����׼����(15X15)����������(19X19)\n");
    char prompt[100];
    sprintf(prompt, "���������̴�С(5~%d)(Ĭ��Ϊ��׼����):\n", MAX_BOARD_SIZE);
    BOARD_SIZE = get_integer_input(prompt, 5, MAX_BOARD_SIZE);
}

/**
 * @brief Set the up game options object
 * ������Ϸѡ��������ֹ��򡢼�ʱ����ʱ������
 */
void setup_game_options()
{
    use_forbidden_moves = get_integer_input("�Ƿ����ý��ֹ��� (1-��, 0-��): ", 0, 1);

    use_timer = get_integer_input("�Ƿ����ü�ʱ�� (1-��, 0-��): ", 0, 1);
    if (use_timer)
    {
        time_limit = get_integer_input("������ÿ�غϵ�ʱ������ (1~60����): ", 1, 60) * 60;
    }
}

/**
 * @brief ȷ���������
 *
 * @param player1
 * @param player2
 * @return int player1 or player2
 */
int determine_first_player(int player1, int player2)
{
    char prompt[100];
    sprintf(prompt, "��ѡ�����ַ� (1 for Player %d, 2 for Player %d): ", player1, player2);
    int first_player_choice = get_integer_input(prompt, 1, 2);
    if (first_player_choice == 1)
    {
        return player1;
    }
    else
    {
        return player2;
    }
}