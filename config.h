/**
 * @file config.h
 * @author ������(3364451258@qq.com��15236416560@163.com��lhy3364451258@outlook.com)
 * @brief ��������Ϸ��������ͷ�ļ�
 * @version 6.0
 * @date 2025-07-10
 *
 * @copyright Copyright (c) 2025
 *
 * @note ���ļ����ж�������������Ϸ�����в������ã�����ͳһ������޸�
 */

#ifndef CONFIG_H
#define CONFIG_H

//---------- ������ز��� ----------//
#define MAX_BOARD_SIZE 25                           // ֧�ֵ�������̳ߴ�
#define MIN_BOARD_SIZE 5                            // ֧�ֵ���С���̳ߴ�
#define DEFAULT_BOARD_SIZE 15                       // Ĭ�����̳ߴ�
#define MAX_STEPS (MAX_BOARD_SIZE * MAX_BOARD_SIZE) // ��Ϸ�����

//---------- ��ұ�ʶ���� ----------//
#define EMPTY 0                                     // ���̿�λ��ʶ
#define PLAYER 1                                    // ��ұ�ʶ (�����˻���սģʽ)
#define AI 2                                        // AI��ʶ (�����˻���սģʽ)
#define PLAYER1 1                                   // ���1��ʶ (����˫�˶�սģʽ)
#define PLAYER2 2                                   // ���2��ʶ (����˫�˶�սģʽ)

//---------- ������������ ----------//
#define INPUT_UNDO -1       // ����
#define INPUT_SAVE -2       // ����
#define INPUT_EXIT -3       // �˳�
#define INPUT_SURRENDER -4  // ����

//---------- ��Ϸ����Ĭ��ֵ ----------//
#define DEFAULT_USE_FORBIDDEN_MOVES false  // Ĭ�ϲ����ý��ֹ���
#define DEFAULT_USE_TIMER 0                // Ĭ�ϲ����ü�ʱ��
#define DEFAULT_TIME_LIMIT 30              // Ĭ��ʱ������Ϊ30��(�ڲ��洢)

//---------- AI���� ----------//
#define DEFAULT_AI_DEPTH 3                 // Ĭ��AI�������
#define DEFAULT_DEFENSE_COEFFICIENT 1.2    // Ĭ�Ϸ���ϵ��

//---------- ������� ----------//
#define DEFAULT_NETWORK_PORT 8888          // Ĭ������˿�
#define MIN_NETWORK_PORT 1024              // ��С����˿�
#define MAX_NETWORK_PORT 65535             // �������˿�
#define NETWORK_TIMEOUT_MS 5000            // ���糬ʱʱ��(����)
#define NETWORK_BUFFER_SIZE 1024           // ���绺������С

//---------- ���ֲ��� ----------//
// �������� - ����calculate_step_score����
#define SCORE_FIVE 0            // ����
#define SCORE_LIVE_FOUR 2000    // ����
#define SCORE_RUSH_FOUR 1000    // ����
#define SCORE_DEAD_FOUR 300     // ����
#define SCORE_LIVE_THREE 500    // ����
#define SCORE_SLEEP_THREE 200   // ����
#define SCORE_DEAD_THREE 80     // ����
#define SCORE_LIVE_TWO 100      // ���
#define SCORE_SLEEP_TWO 40      // �߶�
#define SCORE_DEAD_TWO 15       // ����
#define SCORE_LIVE_ONE 15       // ���ŵ���
#define SCORE_HALF_ONE 8        // �뿪�ŵ���
#define SCORE_DEAD_ONE 2        // ��յ���

// λ�ý���ϵ��
#define POSITION_BONUS_FACTOR 10     // λ�ý�������

// AI�������� - ����evaluate_pos����
#define AI_SCORE_FIVE 1000000        // AI����-����
#define AI_SCORE_LIVE_FOUR 100000    // AI����-����
#define AI_SCORE_RUSH_FOUR 10000     // AI����-����
#define AI_SCORE_DEAD_FOUR 500       // AI����-����
#define AI_SCORE_LIVE_THREE 5000     // AI����-����
#define AI_SCORE_SLEEP_THREE 1000    // AI����-����
#define AI_SCORE_DEAD_THREE 50       // AI����-����
#define AI_SCORE_LIVE_TWO 500        // AI����-���
#define AI_SCORE_SLEEP_TWO 100       // AI����-�߶�
#define AI_SCORE_DEAD_TWO 10         // AI����-����
#define AI_SCORE_LIVE_ONE 50         // AI����-���ŵ���
#define AI_SCORE_HALF_ONE 10         // AI����-�뿪�ŵ���
#define AI_SCORE_DEAD_ONE 1          // AI����-��յ���

// AIλ�ý���ϵ��
#define AI_POSITION_BONUS_FACTOR 50  // AIλ�ý�������

// �����㷨����
#define SEARCH_MAX_SCORE 1000000        // ����������
#define SEARCH_WIN_BONUS 1000000        // ��ʤ��������
#define AI_NEARBY_RANGE 2               // AI�������ڽ���Χ
#define AI_SEARCH_RANGE_THRESHOLD 10    // AI��ʼ����������Χ�Ĳ�����ֵ

// ����Ȩ�ز���
#define TIME_WEIGHT_FACTOR 0.5          // ʱ��Ȩ������
#define WIN_BONUS 2000                  // ʤ����������

// �ļ�·������
#define RECORDS_DIR "records"           // ��¼�ļ�Ŀ¼
#define CONFIG_FILE "gobang_config.ini" // �����ļ�·��
#define MAX_PATH_LENGTH 256             // ���·������

//---------- ���ù��������� ----------//
/**
 * @brief ������Ϸ����
 */
void load_game_config();

/**
 * @brief ������Ϸ����
 */
void save_game_config();

/**
 * @brief ����ΪĬ������
 */
void reset_to_default_config();

/**
 * @brief ��ʾ��ǰ����
 */
void display_current_config();

/**
 * @brief �������̴�С
 */
void config_board_size();

/**
 * @brief ���ý��ֹ���
 */
void config_forbidden_moves();

/**
 * @brief ���ü�ʱ��
 */
void config_timer();

/**
 * @brief �����������
 */
void config_network();

/**
 * @brief ���ù������˵�
 */
void config_management_menu();

//---------- ��������ȫ�ֱ������� ----------// ȫ�ֱ�������������globals.h��

#endif // CONFIG_H