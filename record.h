#ifndef RECORD_H
#define RECORD_H

#include "gobang.h"

// ȫ�ֱ��������ڴ洢�Ծ����֣�ȷ����ս�����͸���ģʽʹ����ͬ������
extern int player1_final_score;
extern int player2_final_score;
extern int scores_calculated;

// --- �������¼ ---
/**
 * @brief ���븴�����̣��ع�������Ϸ
 * @param game_mode ��Ϸģʽ��1Ϊ�˻���2Ϊ˫�ˣ�
 */
void review_process(int game_mode);

/**
 * @brief ����ǰ�Ծּ�¼���浽�ļ�
 * @param filename Ҫ���浽���ļ���
 * @param game_mode ��Ϸģʽ
 * @return 0��ʾ�ɹ�����0��ʾʧ��
 */
int save_game_to_file(const char *filename, int game_mode);

/**
 * @brief ��������Ϸ��¼���߼�
 * @param game_mode ��Ϸģʽ
 */
void handle_save_record(int game_mode);

/**
 * @brief ���ļ�������Ϸ��¼
 * @param filename Ҫ���ص��ļ���
 * @return 0��ʾ�ɹ�����0��ʾʧ��
 */
int load_game_from_file(const char *filename);

#endif // RECORD_H