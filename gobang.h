/**
 * @file gobang.h
 * @brief ��������Ϸ�����߼�ͷ�ļ�
 * @details ��������Ϸ�ĺ������ݽṹ��ȫ�ֱ������궨��ͺ���ԭ�͡�
 * @author ������(3364451258@qq.com��15236416560@163.com��lhy3364451258@outlook.com)
 * @date 2025-07-02
 * @version 4.0
 * @note
 * 1. �������ܣ�
 *    - �����˶Խ��ֹ����֧�֣���ֹ��ҽ�����������߷���
 *    - ��������Ϸ��ʱ�����ܣ�����ÿ�غϵ�˼��ʱ�䡣
 * 2. �����Ż���
 *    - �Ż����������������ܣ������˲���Ҫ�ļ��㡣
 *    - ������ Alpha-Beta ��֦�㷨������� AI ������Ч�ʡ�
 * 3. �û�����Ľ���
 *    - �����������н��棬�ṩ���ѺõĽ������顣
 *    - �����Զ������̴�С��������Ϸ������ԡ�
 * 4. ����ṹ�Ż���
 *    - ����Ϸ�߼����û�������룬��ߴ���Ŀɶ��ԺͿ�ά���ԡ�
 *    - �Ż��˴���ṹ������˴���Ŀɶ��ԺͿ�ά���ԡ�
 * 5. �쳣����
 *    - ���������������쳣������ƣ�ȷ����Ϸ���ȶ��ԡ�
 *    - �޸���һЩ��֪�� bug�������Ϸ���ȶ��ԡ�
 * 6. �ĵ����£�
 *    - �����˴���ע�ͣ�����˴���Ŀɶ��ԡ�
 *    - �������ĵ�����������������ʹ�÷�����ע������ȡ�
 * 7. �汾���ƣ�
 *    - ʹ�� Git ���а汾���ƣ������Ŷ�Э���ʹ������
 * 8. ���ԣ�
 *    - ������ȫ��Ĳ��ԣ�ȷ����Ϸ���ȶ��Ժ͹��ܵ���ȷ�ԡ�
 * 9. ��ԴЭ�飺
 *    - ѡ���� MIT ��ԴЭ�飬�����û�����ʹ�á��޸ĺͷַ����롣
 * 10. �����ߣ�
 *    - ������
 * 11. ��ϵ��Ϣ��
 *    - ��Ŀ��ҳ��[https://github.com/LHY0125/Gobang-Game]
 *    - ��ϵ���䣺[3364451258@qq.com][15236416560@163.com][lhy3364451258@outlook.com]
 */

#ifndef GO_BANG_H
#define GO_BANG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// �궨��
#define MAX_BOARD_SIZE 25                           // ֧�ֵ�������̳ߴ�
#define PLAYER 1                                    // ��ұ�ʶ (�����˻���սģʽ)
#define AI 2                                        // AI��ʶ (�����˻���սģʽ)
#define PLAYER1 1                                   // ���1��ʶ (����˫�˶�սģʽ)
#define PLAYER2 2                                   // ���2��ʶ (����˫�˶�սģʽ)
#define EMPTY 0                                     // ���̿�λ��ʶ
#define MAX_STEPS (MAX_BOARD_SIZE * MAX_BOARD_SIZE) // ��Ϸ�����

// ȫ�ֱ���
extern int BOARD_SIZE;                              // ��ǰʵ��ʹ�õ����̳ߴ�
extern int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];   // �洢����״̬�Ķ�ά����
extern int step_count;                              // ��ǰ��Ϸ���ܲ���
extern bool use_forbidden_moves;                    // �Ƿ����ý��ֹ���ı�־
extern int use_timer;                               // �Ƿ����ü�ʱ���ı�־
extern int time_limit;                              // ÿ�غϵ�ʱ�����ƣ��룩
extern const int direction[4][2];                   // �����ĸ�������������ˮƽ����ֱ����б����б

// ���ݽṹ
/**
 * @brief ��¼һ�������ϸ��Ϣ
 */
typedef struct
{
    int player; // ִ�иò�����ұ�ʶ
    int x;      // ���ӵ������� (0-based)
    int y;      // ���ӵ������� (0-based)
} Step;

extern Step steps[MAX_STEPS]; // ���ڴ洢��Ϸ��ÿһ���������

/**
 * @brief �洢���ض����������������Ե���Ϣ
 * @details �����������Σ������жϻ��������ĵȹؼ���̬��
 */
typedef struct
{
    int continuous_chess; // ����ͬɫ���ӵ�����
    bool check_start;     // �������е���ʼ���Ƿ�Ϊ��λ�����Ƿ񿪷ţ�
    bool check_end;       // �������е�ĩβ���Ƿ�Ϊ��λ�����Ƿ񿪷ţ�
} DirInfo;

// ����ԭ��


// --- ��Ϸ�����߼� ---
/**
 * @brief ���ָ�������Ƿ�Ϊ��Ч���ӵ㣨����������Ϊ�գ�
 * @param x ������������ (0-based)
 * @param y ������������ (0-based)
 * @return ��λ����Ч��Ϊ���򷵻�true�����򷵻�false
 */
bool have_space(int x, int y);

/**
 * @brief �ж�һ�������Ƿ�Ϊ����
 * @param x ���ӵ������� (0-based)
 * @param y ���ӵ������� (0-based)
 * @param player ��ǰ��ҵı�ʶ
 * @return ����ǽ����򷵻�true�����򷵻�false
 */
bool is_forbidden_move(int x, int y, int player);


/**
 * @brief ִ��һ��������Ӳ���
 * @param x ���ӵ������� (0-based)
 * @param y ���ӵ������� (0-based)
 * @param player ��ǰ��ҵı�ʶ
 * @return �����ӳɹ��򷵻�true������λ����Ч��ռ�ã�����false
 */
bool player_move(int x, int y, int player);

/**
 * @brief �������ض������ϵ�����������Ϣ
 * @param x ��ʼ���������
 * @param y ��ʼ���������
 * @param dx x��������� (-1, 0, or 1)
 * @param dy y��������� (-1, 0, or 1)
 * @param player ��ұ�ʶ
 * @return ����һ����������������Ϣ�� DirInfo �ṹ��
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player);

/**
 * @brief �����ĳ�����Ӻ󣬸�����Ƿ��ʤ
 * @param x ���ӵ������� (0-based)
 * @param y ���ӵ������� (0-based)
 * @param player ��ǰ��ҵı�ʶ
 * @return �����ʤ�򷵻�true�����򷵻�false
 */
bool check_win(int x, int y, int player);

/**
 * @brief ���幦�ܣ�����ָ������
 * @param steps_to_undo Ҫ�����Ĳ�����ÿ������˫����һ�����ӣ�
 * @return ������ɹ��򷵻�true�����򷵻�false
 */
bool return_move(int steps_to_undo);

/**
 * @brief ���㲢����һ����ĵ÷�
 * @param x ���ӵ�������
 * @param y ���ӵ�������
 * @param player ��ұ�ʶ
 * @return �ò���ĵ÷�
 */
int calculate_step_score(int x, int y, int player);

#endif // GO_BANG_H