#ifndef INIT_BOARD_H
#define INIT_BOARD_H

#include "gobang.h"

// --- ��Ϸ��ʼ�� ---
/**
 * @brief ��ʼ�����̣�������λ������Ϊ��(EMPTY)
 */
void empty_board();

/**
 * @brief ����ǰ����״̬��ӡ������̨
 */
void print_board();

/**
 * @brief ���õ�ǰ��Ϸ�����̴�С
 */
void setup_board_size();

/**
 * @brief ������Ϸѡ����Ƿ����ý��֡���ʱ����
 */
void setup_game_options();

/**
 * @brief �����������
 * @param player1 ���1�ı�ʶ
 * @param player2 ���2�ı�ʶ
 * @return ����������ҵı�ʶ
 */
int determine_first_player(int player1, int player2);

#endif // INIT_H