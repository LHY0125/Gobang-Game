#ifndef GO_BANG_H
#define GO_BANG_H

#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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