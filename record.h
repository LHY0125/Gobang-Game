#ifndef RECORD_H
#define RECORD_H

#include "gobang.h"

// --- �������¼���� ---
/**
 * @brief ���븴�����̣��ع�������Ϸ
 * @param game_mode ��Ϸģʽ��1Ϊ�˻���ս��2Ϊ˫�˶�ս��
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
 * @return ��Ϸģʽ��1��2����0��ʾʧ��
 */
int load_game_from_file(const char *filename);

/**
 * @brief ������Ϸ����
 */
void calculate_game_scores();

/**
 * @brief ��ʾ��Ϸ���ֽ����MVP��ѡ
 * @param game_mode ��Ϸģʽ��1-�˻���ս��2-˫�˶�ս��
 */
void display_game_scores(int game_mode);

#endif // RECORD_H