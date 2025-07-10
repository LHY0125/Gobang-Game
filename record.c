#include "record.h"
#include "game_mode.h"
#include "gobang.h"
#include "init_board.h"
#include "ui.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

/**
 * @brief ������Ϸȫ���̲�չʾ����
 * @note ʵ������:
 * 1. ��ʼ����ʱ��������
 * 2. ������˳��������ÿ������
 * 3. ÿ����ʾ:
 *    - ��ǰ����/�ܲ���
 *    - ���ӷ�(���/AI)
 *    - ����λ��(1-based����)
 *    - ��ǰ����״̬
 * 4. ͨ���û���Enter�����Ʋ���ǰ��
 * 5. ���̽������Զ��������ֻ���:
 *    - ����˫������
 *    - ��ʾ�÷�
 *    - ��ѡMVP
 * @note ����ϸ��:
 * - ʹ�ö�����ʱ���̱���Ӱ������Ϸ״̬
 * - ������ʾת��Ϊ1-based�����û����
 * - �������뻺���������ֹ��������
 * - ���ֻ��ڵ���calculate_final_score()����
 */
void review_process(int game_mode)
{
    int review_choice = get_integer_input("�Ƿ�Ҫ���̱��ֱ���? (1-��, 0-��): ", 0, 1);
    
    // ���������δ���㣬���������
    if (!scores_calculated)
    {
        calculate_game_scores();
    }
    else
    {
        // �����Ѵ��ļ��м��أ�ֱ��ʹ��
        printf("�Ӽ�¼�ļ��м�����������\n");
    }
    
    if (review_choice == 1)
    {
        printf("\n===== ���̼�¼(�ܲ�����%d) =====\n", step_count);
        // ������뻺����
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        // ������ʱ��������
        int temp_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
        memset(temp_board, EMPTY, sizeof(temp_board)); // ��ʼ��Ϊ������

        // ��������Ϸ����
        for (int i = 0; i < step_count; i++)
        {
            Step s = steps[i];               // ��ȡ��ǰ����
            temp_board[s.x][s.y] = s.player; // ����ʱ����������

            // ��ӡ��ǰ������Ϣ
            // ������Ϸģʽ��ʾ��ͬ�ı���������Ϣ
            if (game_mode == 1)
            {
                // �˻���ս
                printf("\n===== �������˻���ս(%dX%d����) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    ��%d��/%d��: %s ������(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER) ? "���" : "AI",
                       s.x + 1, s.y + 1);
            }
            else
            {
                // ˫�˶�ս
                printf("\n===== ������˫�˶�ս(%dX%d����) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    ��%d��/%d��: %s ������(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER1) ? "���1(����)" : "���2(����)",
                       s.x + 1, s.y + 1);
            }

            // ��ӡ��ǰ��������
            printf("  ");
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                printf("%2d", col + 1); // �к�
            }           
            printf("\n");

            for (int row = 0; row < BOARD_SIZE; row++)
            {
                printf("%2d ", row + 1); // �к�
                for (int col = 0; col < BOARD_SIZE; col++)
                {
                    if (temp_board[row][col] == PLAYER || temp_board[row][col] == PLAYER1)
                    {
                        printf("x ");
                    }
                    else if (temp_board[row][col] == AI || temp_board[row][col] == PLAYER2)
                    {
                        printf("�� ");
                    }
                    else
                    {
                        printf("�� ");
                    }
                }
                printf("\n"); // �н�������
            }

            // ����������һ�����ȴ��û���������
            if (i < step_count - 1)
            {
                printf("\n��Enter������һ��...");
                while (getchar() != '\n')
                    ; // �ȴ��س�
            }
        }
        
        // ��ʾʤ�������ֱ��ʹ���ļ��е���Ϣ��
        printf("\n===== �Ծֽ�� =====");
        if (strcmp(winner_info, "��һ�ʤ") == 0)
        {
            printf("\n? ��ϲ����һ�ʤ��\n");
        }
        else if (strcmp(winner_info, "AI��ʤ") == 0)
        {
            printf("\n? AI��ʤ��\n");
        }
        else if (strcmp(winner_info, "���1��ʤ") == 0)
        {
            printf("\n? ��ϲ�����1(����)��ʤ��\n");
        }
        else if (strcmp(winner_info, "���2��ʤ") == 0)
        {
            printf("\n? ��ϲ�����2(����)��ʤ��\n");
        }
        else
        {
            printf("\n?? �Ծ�ƽ�ֻ�δ���\n");
        }
        
        printf("\n���̽�������Enter�鿴����...");
        getchar(); // �ȴ��û�����
    }

    // ��ʾ���ֽ��
    display_game_scores(game_mode);

    getchar();
}

/**
 * @brief ������Ϸ����
 */
void calculate_game_scores()
{
    // ����˫������
    player1_final_score = 0;
    player2_final_score = 0;

    // �������в������ۻ�ÿһ���ĵ÷֣����ڲ���Ȩ�ظ���
    for (int i = 0; i < step_count; i++)
    {
        // ����ʱ��Ȩ�����ӣ�����Խ����Ȩ��Խ��
        double time_weight = 1.0 + (double)i / step_count * TIME_WEIGHT_FACTOR; // ���Ĳ���Ȩ���ǿ�ʼ�����(1+TIME_WEIGHT_FACTOR)��
        
        if (steps[i].player == PLAYER || steps[i].player == PLAYER1)
        {
            player1_final_score += (int)(calculate_step_score(steps[i].x, steps[i].y, steps[i].player) * time_weight);
        }
        else
        {
            player2_final_score += (int)(calculate_step_score(steps[i].x, steps[i].y, steps[i].player) * time_weight);
        }
    }
    
    // ʤ����Ȩ����ʤ����ö�������ֽ���
    if (step_count > 0)
    {
        Step last_step = steps[step_count - 1];
        if (check_win(last_step.x, last_step.y, last_step.player))
        {
            // ��ʤ����ö��⽱������
            if (last_step.player == PLAYER || last_step.player == PLAYER1)
            {
                player1_final_score += WIN_BONUS; // ��ʤ����
            }
            else
            {
                player2_final_score += WIN_BONUS; // ��ʤ����
            }
        }
    }
    
    scores_calculated = 1; // ��������Ѽ���
}

/**
 * @brief ��ʾ��Ϸ���ֽ����MVP��ѡ
 * @param game_mode ��Ϸģʽ(1-�˻���ս, 2-˫�˶�ս)
 */
void display_game_scores(int game_mode)
{
    printf("\n===== �Ծ����� =====\n");
    double sum_score = (long double)player1_final_score + (long double)player2_final_score;

    if (sum_score > 0)
    {
        if (game_mode == 1)
        {
            printf("��ҵ÷�: %d, ռ��: %.2f%%\n",
                   player1_final_score, (double)player1_final_score * 100.0 / sum_score);
            printf("AI�÷�: %d, ռ��: %.2f%%\n",
                   player2_final_score, (double)player2_final_score * 100.0 / sum_score);
        }
        else
        {
            printf("���1(����)�÷�: %d, ռ��: %.2f%%\n",
                   player1_final_score, (double)player1_final_score * 100.0 / sum_score);
            printf("���2(����)�÷�: %d, ռ��: %.2f%%\n",
                   player2_final_score, (double)player2_final_score * 100.0 / sum_score);
        }
    }
    else
    {
        if (game_mode == 1)
        {
            printf("��ҵ÷�: %d\n", player1_final_score);
            printf("AI�÷�: %d\n", player2_final_score);
        }
        else
        {
            printf("���1(����)�÷�: %d\n", player1_final_score);
            printf("���2(����)�÷�: %d\n", player2_final_score);
        }
        printf("ע: ˫���÷־�Ϊ0���޷�����ռ��\n");
    }

    // ��ѡMVP
    if (player1_final_score > player2_final_score)
    {
        printf("\nMVP: %s (���� %d ��)\n", (game_mode == 1) ? "���" : "���1(����)", player1_final_score - player2_final_score);
    }
    else if (player2_final_score > player1_final_score)
    {
        printf("\nMVP: %s (���� %d ��)\n", (game_mode == 1) ? "AI" : "���2(����)", player2_final_score - player1_final_score);
    }
    else
    {
        printf("\n˫���ƾ����У�\n");
    }
}

/**
 * @brief ������Ϸ������ļ�¼����
 * @return int ����״̬��(0-�ɹ�, 1-Ŀ¼����ʧ��, 2-�ļ���ʧ��, 3-�ļ�д��ʧ��)
 */
void handle_save_record(int game_mode)
{
    int save_choice = 0;
    printf("===== ��Ϸ���� =====\n");
    printf("�Ƿ񱣴���Ϸ��¼? (1-��, 0-��): ");
    scanf("%d", &save_choice);

    if (save_choice == 1)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char filename[256];
        strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.csv", t);

        int save_status = save_game_to_file(filename, game_mode);
        switch (save_status)
        {
        case 0: // �ɹ�
            printf("\n��Ϸ��¼�ѳɹ�������: %s (CSV��ʽ)\n", filename);
            printf("������ʹ������������и���: .\\gobang.exe -l %s\n", filename);
            printf("CSV��ʽ�ļ�����ֱ����Excel�򿪲鿴�ͷ���\n");
            break;
        case 1: // Ŀ¼����ʧ��
            printf("\n��Ϸ��¼����ʧ��: �޷����� 'records' Ŀ¼��\n");
            printf("��������Ƿ�����㹻��д��Ȩ�޻���̿ռ��Ƿ���㡣\n");
            break;
        case 2: // �ļ���ʧ��
            printf("\n��Ϸ��¼����ʧ��: �޷���·�� '%s' �����ļ���\n", filename);
            printf("����·���Ƿ���Ч�Լ������Ƿ����д��Ȩ�ޡ�\n");
            break;
        case 3: // �ļ�д��ʧ��
            printf("\n��Ϸ��¼����ʧ��: д���ļ�ʱ��������\n");
            printf("������̿ռ��Ƿ�������\n");
            break;
        default:
            printf("\n��Ϸ��¼����ʧ��: ����δ֪����\n");
            break;
        }
    }
}

/**
 * @brief ����ǰ��Ϸ��¼���浽�ļ�
 * @param filename Ҫ������ļ���
 * @return int ������:
 *   0: �ɹ�
 *   1: Ŀ¼����ʧ��
 *   2: �ļ���ʧ��
 *   3: �ļ�д��ʧ��
 */
int save_game_to_file(const char *filename, int game_mode)
{
    // ����recordsĿ¼(���������)
    struct stat st = {0};
    if (stat("records", &st) == -1)
    {
        if (mkdir("records") != 0)
        {
            // ����Ƿ�Ŀ¼�Ѵ���(���߳�����¿��ܱ������̴߳���)
            if (stat("records", &st) == -1)
            {
#ifdef _WIN32
                printf("�����޷�����recordsĿ¼\n");
                printf("����ԭ��\n");
                printf("1. û��д��Ȩ�� - �볢���Թ���Ա�������\n");
                printf("2. �����������ֹ - ���鰲ȫ�������\n");
                printf("3. ·����Ч - ���鹤��Ŀ¼\n");
#else
                perror("����Ŀ¼ʧ��");
#endif
                return 1; // Ŀ¼����ʧ��
            }
        }
    }

    // ���ļ�
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "w");
    if (!file)
    {
        return 2; // �ļ���ʧ��
    }

    // �ж�ʤ�����
    strcpy(winner_info, "ƽ�ֻ�δ���");
    if (step_count > 0)
    {
        Step last_step = steps[step_count - 1];
        if (check_win(last_step.x, last_step.y, last_step.player))
        {
            if (game_mode == 1)
            {
                // �˻���ս
                if (last_step.player == PLAYER)
                {
                    strcpy(winner_info, "��һ�ʤ");
                }
                else
                {
                    strcpy(winner_info, "AI��ʤ");
                }
            }
            else
            {
                // ˫�˶�ս
                if (last_step.player == PLAYER1)
                {
                    strcpy(winner_info, "���1��ʤ");
                }
                else
                {
                    strcpy(winner_info, "���2��ʤ");
                }
            }
        }
    }
    
    // д��CSV�ļ�ͷ��
    if (fprintf(file, "��Ϸģʽ,���̴�С,���1�÷�,���2�÷�,�Ծֽ��\n%d,%d,%d,%d,%s\n\n", game_mode, BOARD_SIZE, player1_final_score, player2_final_score, winner_info) < 0)
    {
        fclose(file);
        return 3; // �ļ�д��ʧ��
    }
    
    // д��CSV��ͷ
    if (fprintf(file, "����,���,������,������\n") < 0)
    {
        fclose(file);
        return 3; // �ļ�д��ʧ��
    }

    // д���������Ӳ��裨CSV��ʽ��
    for (int i = 0; i < step_count; i++)
    {
        if (fprintf(file, "%d,%d,%d,%d\n", i+1, steps[i].player, steps[i].x+1, steps[i].y+1) < 0)
        {
            fclose(file);
            return 3; // �ļ�д��ʧ��
        }
    }

    if (fclose(file) != 0)
    {
        return 3; // �ļ��ر�/д��ʧ��
    }

    return 0; // �ɹ�
}

/**
 * @brief ���ļ�������Ϸ��¼
 * @param filename Ҫ���ص��ļ���
 * @return true ���سɹ�
 * @return false ����ʧ��
 */
int load_game_from_file(const char *filename)
{
    // ���ļ�
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "r");
    if (!file)
    {
        return false;
    }

    // ����CSV�ļ�ͷ����
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file) == NULL) // ����"��Ϸģʽ,���̴�С"
    {
        fclose(file);
        return 0;
    }

    // ��ȡ��Ϸģʽ�����̴�С�����ֽ��
    int game_mode, size;
    
    // ���Զ�ȡ�¸�ʽ������ʤ����Ϣ��
    int read_count = fscanf(file, "%d,%d,%d,%d,%49s", &game_mode, &size, &player1_final_score, &player2_final_score, winner_info);
    
    if (read_count == 4)
    {
        // �ɸ�ʽ�ļ���û��ʤ����Ϣ
        strcpy(winner_info, "δ֪");
    }
    else if (read_count != 5)
    {
        // �ļ���ʽ����
        fclose(file);
        return 0;
    }
    
    if (game_mode != 1 && game_mode != 2)
    {
        fclose(file);
        return 0; // ��Ч����Ϸģʽ
    }
    if (size < 5 || size > MAX_BOARD_SIZE)
    {
        fclose(file);
        return false;
    }
    
    // ���������Ѽ����־
    scores_calculated = 1;

    // �������кͱ�ͷ��
    fgets(buffer, sizeof(buffer), file); // ��������
    fgets(buffer, sizeof(buffer), file); // ��������
    fgets(buffer, sizeof(buffer), file); // ����"����,���,������,������"

    // ��ʼ������
    BOARD_SIZE = size;
    empty_board();

    // ��ȡ�������Ӳ���
    step_count = 0;
    int step_num; // ���ڴ洢����������ʹ��
    while (fscanf(file, "%d,%d,%d,%d", &step_num, &steps[step_count].player, &steps[step_count].x, &steps[step_count].y) == 4)
    {
        // ��1-based����ת��Ϊ0-based����
        steps[step_count].x--;
        steps[step_count].y--;
        step_count++;
    }

    fclose(file);
    return game_mode;
}