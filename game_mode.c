#include "gobang.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <conio.h>
#endif

/**
 * @brief 处理玩家回合
 * 
 * @param current_player 
 * @return true 
 * @return false 
 */
bool handle_player_turn(int current_player)
{
    int x, y;
    char input[10];
    time_t start_time, end_time;
    if (use_timer)
    {
        time(&start_time);
    }

    if (current_player == 1)
    {
        printf("\n玩家, 请输入落子坐标(行 列，1~%d)，或输入R/r悔棋:", BOARD_SIZE);
    }
    else
    {
        printf("\n玩家%d, 请输入落子坐标(行 列，1~%d)，或输入R/r悔棋:", current_player, BOARD_SIZE);
    }

    while (1)
    {
        if (use_timer)
        {
            time(&end_time);
            if (difftime(end_time, start_time) > time_limit)
            {
                if (current_player == 1)
                {
                    printf("\n玩家超时, 对方获胜！\n");
                }
                else
                {
                    printf("\n玩家%d超时, 对方获胜！\n", current_player);
                }
                return false; // 超时，返回false表示回合失败
            }
        }
        if (_kbhit())
        {
            scanf("%s", input);
            break;
        }
        Sleep(100); // a small delay to prevent high CPU usage
    }

    if (input[0] == 'r' || input[0] == 'R')
    {
        int steps_to_undo;
        printf("请输入要悔棋的步数(AI会同样悔棋): ");
        steps_to_undo = get_integer_input("", 1, step_count / 2);
        int effective_steps = (current_player == PLAYER) ? steps_to_undo * 2 : steps_to_undo;
        if (return_move(effective_steps))
        {
            printf("成功悔棋 %d 步！\n", steps_to_undo);
            print_board();
        }
        else
        {
            printf("无法悔棋！\n");
        }
        return true; // 悔棋操作后，回合算作成功，但不进行落子
    }

    if (sscanf(input, "%d", &x) != 1)
    {
        printf("无效输入，请输入数字坐标。");
        return true; // 输入无效，但回合继续
    }
    if (scanf("%d", &y) != 1)
    {
        printf("无效输入，请输入数字坐标。");
        while (getchar() != '\n')
            ;
        return true; // 输入无效，但回合继续
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
        if (current_player == 1)
        {
            printf("\n玩家获胜！\n");
        }
        else
        {
            printf("\n玩家%d获胜！\n", current_player);
        }
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
    setup_game_options();

    // AI对战模式
    setup_board_size();
    int AI_DEPTH = 3;
    AI_DEPTH = get_integer_input("请选择AI难度(1~5), 数字越大越强，注意数字越大AI思考时间越长！):", 1, 5);

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
    int review_choice;
    review_choice = get_integer_input("是否要复盘本局比赛? (1-是, 0-否): ", 0, 1);
    if (review_choice == 1)
    {
        review_process();
    }
end_game:
end_pvp_game:
    handle_save_record();
}

/**
 * @brief 运行双人对战模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_pvp_game()
{
    setup_game_options();

    // 双人对战模式
    setup_board_size();
    empty_board();
    int current_player = determine_first_player(PLAYER3, PLAYER4);
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
            current_player = (current_player == PLAYER3) ? PLAYER4 : PLAYER3;
        }
    }
    printf("===== 游戏结束 =====\n");
    int review_choice;
    review_choice = get_integer_input("是否要复盘本局比赛? (1-是, 0-否): ", 0, 1);
    if (review_choice == 1)
    {
        review_process();
    }
    handle_save_record();
}

/**
 * @brief 运行复盘模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_review_mode()
{
    // 复盘模式
    char filename[256];
    printf("请输入复盘文件地址: ");
    scanf("%s", filename);
    if (load_game_from_file(filename))
    {
        printf("成功加载历史记录: %s\n", filename);
        review_process();
    }
    else
    {
        printf("加载历史记录失败: %s\n", filename);
        exit(1);
    }
}