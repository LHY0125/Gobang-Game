#include "game_mode.h"
#include "init_board.h"
#include "gobang.h"
#include "ai.h"
#include "record.h"
#include "config.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>

// 引用record.h中定义的全局变量
extern int scores_calculated;
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <conio.h>
#endif

/**
 * @brief 从用户获取整数输入
 *
 * @param prompt 提示信息
 * @param min 最小值
 * @param max 最大值
 * @return int 用户输入的整数
 */
int get_integer_input(const char *prompt, int min, int max)
{
    int value;
    int result;
    char ch;

    while (1)
    {
        printf("%s", prompt);
        result = scanf("%d", &value);

        if (result == 1 && value >= min && value <= max)
        {
            // 清除输入缓冲区中剩余的字符
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            return value;
        }
        else
        {
            // 清除无效输入
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("输入无效，请输入一个介于 %d 和 %d 之间的整数。\n", min, max);
        }
    }
}

/**
 * @brief 处理玩家回合
 *
 * @param current_player
 * @return true
 * @return false
 */
bool parse_player_input(int *x, int *y)
{
    char input[10];

    while (1)
    {
        if (_kbhit())
        {
            scanf("%s", input);
            break;
        }
        Sleep(100); // 短暂延迟以防止CPU占用过高
    }

    if (sscanf(input, "%d", x) == 1)
    {
        // 成功解析第一个数字，现在解析第二个
        if (scanf("%d", y) != 1)
        {
            // 如果第二个数字不可用，则检查特殊命令
            if (*x == INPUT_UNDO)
            {
                int steps_to_undo;
                steps_to_undo = get_integer_input("请输入要悔棋的步数(双方各退步数相同): ", 1, step_count / 2);
                if (return_move(steps_to_undo * 2))
                {
                    printf("成功悔棋，双方各退 %d 步！\n", steps_to_undo);
                    print_board();
                }
                else
                {
                    printf("无法悔棋！\n");
                }
                return false; // 特殊命令
            }
            else if (*x == INPUT_SAVE)
            {
                // ... 处理保存 ...
                return false; // 特殊命令
            }
            else if (*x == INPUT_EXIT)
            {
                // ... 处理退出 ...
                return false; // 特殊命令
            }
            printf("无效输入，请输入两个数字坐标。");
            while (getchar() != '\n')
                ;
            return false; // 无效输入
        }
    }
    else
    {
        // sscanf失败，检查特殊命令
        if (input[0] == 'r' || input[0] == 'R')
        {
            int steps_to_undo;
            steps_to_undo = get_integer_input("请输入要悔棋的步数(双方各退步数相同): ", 1, step_count / 2);
            if (return_move(steps_to_undo * 2))
            {
                printf("成功悔棋，双方各退 %d 步！\n", steps_to_undo);
                print_board();
            }
            else
            {
                printf("无法悔棋！\n");
            }
            return false; // 特殊命令
        }
        else if (input[0] == 's' || input[0] == 'S')
        {
            *x = INPUT_SURRENDER;
            int confirm = get_integer_input("确认认输？(1:是/0:否): ", 0, 1);
            if (confirm)
            {
                printf("玩家选择认输！\n");
                return true; // 返回认输命令
            }
            else
            {
                printf("取消认输！\n");
                return false; // 取消认输
            }
        }
        printf("无效输入，请输入数字坐标、'r'悔棋或's'认输。");
        return false; // 无效输入
    }
    return true; // 有效坐标
}

/**
 * @brief 处理玩家回合
 * @param current_player 当前玩家
 * @return true
 */
bool handle_player_turn(int current_player)
{
    int x, y;
    time_t start_time, end_time;
    if (use_timer)
    {
        time(&start_time);
    }

    printf("\n玩家%d, 请输入落子坐标(行 列，1~%d)，或输入R/r悔棋，S/s认输:", current_player, BOARD_SIZE);

    while (1)
    {
        if (use_timer)
        {
            time(&end_time);
            if (difftime(end_time, start_time) > time_limit)
            {
                printf("\n玩家%d超时, 对方获胜！\n", current_player);
                return false; // Timeout
            }
        }

        if (parse_player_input(&x, &y))
        {
            if (x == INPUT_SURRENDER)
            {
                printf("\n玩家%d选择认输，对方获胜！\n", current_player);
                return false; // 游戏结束，认输
            }
            break; // 收到有效输入
        }
        else
        {
            // 已处理特殊命令或无效输入，继续循环
            return true;
        }
    }

    x--;
    y--;

    if (!player_move(x, y, current_player))
    {
        printf("坐标无效！请重新输入。\n");
        return true; // 坐标无效，但回合继续
    }
    print_board();

    if (check_win(x, y, current_player))
    {
        printf("\n玩家%d获胜！\n", current_player);
        return false; // 游戏结束
    }

    return true; // 成功落子
}

/**
 * @brief 运行AI游戏
 * @note 从文件中加载历史记录并进行复盘
 * @param AI_DEPTH AI的搜索深度
 */
void run_ai_game()
{
    // 重置评分计算标志，确保每局游戏都会重新计算评分
    scores_calculated = 0;
    
    setup_game_options();

    // AI对战模式
    setup_board_size();
    int AI_DEPTH = 3;
    AI_DEPTH = get_integer_input("请选择AI难度(1~5), 数字越大越强，注意数字越大AI思考时间越长！):", 1, 5);

    /**
     * @brief AI的防守系数，系数越大越倾向于防守
     * @note 1~1.2
     *       2~1.3
     *       3~1.4
     *       4~1.5
     *       5~1.6
     */
    defense_coefficient = 1.2 + (AI_DEPTH - 1) * 0.1;

    empty_board();
    int current_player = determine_first_player(PLAYER, AI);
    print_board();

    while (1)
    {
        if (current_player == PLAYER)
        {
            int old_step_count = step_count;
            if (!handle_player_turn(current_player))
            {
                break; // 游戏结束或超时
            }
            if (step_count > old_step_count)
            {
                current_player = AI;
            }
        }
        else
        {
            printf("\nAI思考中...\n");
            time_t start_time, end_time;
            if (use_timer)
            {
                time(&start_time);
            }

            ai_move(AI_DEPTH);

            if (use_timer)
            {
                time(&end_time);
                if (difftime(end_time, start_time) > time_limit)
                {
                    printf("\nAI超时, 玩家获胜！\n");
                    break;
                }
            }
            print_board();
            Step last_step = steps[step_count - 1];
            if (check_win(last_step.x, last_step.y, AI))
            {
                printf("\nAI获胜！\n");
                break;
            }
            current_player = PLAYER;
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            break;
        }
    }
    printf("===== 游戏结束 =====\n");
    review_process(1);     // 1 for AI mode
    handle_save_record(1); // 1 for AI mode
}

/**
 * @brief 运行双人对战模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_pvp_game()
{
    // 重置评分计算标志，确保每局游戏都会重新计算评分
    scores_calculated = 0;
    
    setup_game_options();

    // 双人对战模式
    setup_board_size();
    empty_board();
    int current_player = determine_first_player(PLAYER1, PLAYER2);
    print_board();

    while (1)
    {
        int old_step_count = step_count;
        if (!handle_player_turn(current_player))
        {
            break; // 游戏结束或超时
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            break;
        }

        if (step_count > old_step_count)
        {
            current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
        }
    }
    printf("===== 游戏结束 =====\n");
    review_process(2);     // 2 for PvP mode
    handle_save_record(2); // 2 for PvP mode
}

/**
 * @brief 运行复盘模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_review_mode()
{
    char filename[100];
    char record_files[100][100];
    int file_count = 0;

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("records\\*", &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcpy(record_files[file_count++], ffd.cFileName);
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
    }
#endif

    if (file_count > 0)
    {
        printf("发现以下复盘文件:\n");
        for (int i = 0; i < file_count; i++)
        {
            printf("%d. %s\n", i + 1, record_files[i]);
        }

        char prompt[150];
        sprintf(prompt, "请输入复盘文件编号(1-%d)，或输入0以手动输入文件名: ", file_count);
        int choice = get_integer_input(prompt, 0, file_count);

        if (choice > 0)
        {
            strcpy(filename, record_files[choice - 1]);
        }
        else
        {
            printf("请输入完整文件名: ");
            scanf("%s", filename);
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;

            int possible_choice = atoi(filename);
            if (possible_choice > 0 && possible_choice <= file_count)
            {
                strcpy(filename, record_files[possible_choice - 1]);
            }
        }
    }
    else
    {
        printf("未找到任何复盘文件，请输入复盘文件地址: ");
        scanf("%s", filename);
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    int game_mode = load_game_from_file(filename);
    if (game_mode != 0)
    {
        if (game_mode == 1)
        {
            printf("加载AI对战模式复盘文件成功！\n");
        }
        else if (game_mode == 2)
        {
            printf("加载双人对战模式复盘文件成功！\n");
        }
        review_process(game_mode);
    }
    else
    {
        printf("加载复盘文件失败！可能是旧版本文件格式或文件损坏\n");
    }
}