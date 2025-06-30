#include "game_mode.h"
#include "gobang.h"
#include "ai.h"
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
 * @brief ????????????????
 *
 * @param prompt ??????
 * @param min ?????
 * @param max ????
 * @return int ????????????
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
            // ????????????????????
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            return value;
        }
        else
        {
            // ???????????
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("??????????????????????? %d ?? %d ??????????\n", min, max);
        }
    }
}

/**
 * @brief ?????????
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
        Sleep(100);
    }

    if (sscanf(input, "%d", x) == 1)
    {
        if (scanf("%d", y) != 1)
        {
            if (*x == INPUT_UNDO)
            {
                int steps_to_undo;
                printf("???????????????(??????????): ");
                steps_to_undo = get_integer_input("", 1, step_count / 2);
                if (return_move(steps_to_undo * 2))
                {
                    printf("??????????????? %d ????\n", steps_to_undo);
                    print_board();
                }
                else
                {
                    printf("????????\n");
                }
                return false;
            }
            else if (*x == INPUT_SAVE)
            {
                return false;
            }
            else if (*x == INPUT_EXIT)
            {
                return false;
            }
            printf("??????????????????????????");
            while (getchar() != '\n')
                ;
            return false;
        }
    }
    else
    {
        if (input[0] == 'r' || input[0] == 'R')
        {
            int steps_to_undo;
            printf("???????????????(??????????): ");
            steps_to_undo = get_integer_input("", 1, step_count / 2);
            if (return_move(steps_to_undo * 2))
            {
                printf("??????????????? %d ????\n", steps_to_undo);
                print_board();
            }
            else
            {
                printf("????????\n");
            }
            return false;
        }
        printf("???????????????????????'r'????^");
        return false;
    }
    return true;
}

bool handle_player_turn(int current_player)
{
    int x, y;
    time_t start_time, end_time;
    if (use_timer)
    {
        time(&start_time);
    }

    printf("\n???%d, ??????????????(?? ????1~%d)????????R/r????:", current_player, BOARD_SIZE);

    while (1)
    {
        if (use_timer)
        {
            time(&end_time);
            if (difftime(end_time, start_time) > time_limit)
            {
                printf("\n???%d???, ????????\n", current_player);
                return false;
            }
        }

        if (parse_player_input(&x, &y))
        {
            break;
        }
        else
        {
            return true;
        }
    }

    x--;
    y--;

    if (!player_move(x, y, current_player))
    {
        printf("????????????????????\n");
        return true; // ??????????????????
    }
    print_board();

    if (check_win(x, y, current_player))
    {
        printf("\n???%d?????\n", current_player);
        return false; // ???????
    }

    return true; // ???????
}

/**
 * @brief ????AI???
 * @note ???????????????????????????
 * @param AI_DEPTH AI?????????
 */
void run_ai_game()
{
    setup_game_options();

    // AI?????
    setup_board_size();
    int AI_DEPTH = 3;
    AI_DEPTH = get_integer_input("?????AI???(1~5), ?????????????????????AI???????????):", 1, 5);

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
                break; // ??????????
            }
            if (step_count > old_step_count)
            {
                current_player = AI;
            }
        }
        else
        {
            printf("\nAI?????...\n");
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
                    printf("\nAI???, ???????\n");
                    break;
                }
            }
            print_board();
            Step last_step = steps[step_count - 1];
            if (check_win(last_step.x, last_step.y, AI))
            {
                printf("\nAI?????\n");
                break;
            }
            current_player = PLAYER;
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n????\n");
            break;
        }
    }
    printf("===== ??????? =====\n");
    int review_choice;
    review_choice = get_integer_input("??????????????? (1-??, 0-??): ", 0, 1);
    if (review_choice == 1)
    {
        review_process(1); // 1 for AI mode
    }
    handle_save_record(1); // 1 for AI mode
}

/**
 * @brief ???????????
 * @note ???????????????????????????
 */
void run_pvp_game()
{
    setup_game_options();

    // ???????
    setup_board_size();
    empty_board();
    int current_player = determine_first_player(PLAYER1, PLAYER2);
    print_board();

    while (1)
    {
        int old_step_count = step_count;
        if (!handle_player_turn(current_player))
        {
            break; // ??????????
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n????\n");
            break;
        }

        if (step_count > old_step_count)
        {
            current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
        }
    }
    printf("===== ??????? =====\n");
    int review_choice;
    review_choice = get_integer_input("??????????????? (1-??, 0-??): ", 0, 1);
    if (review_choice == 1)
    {
        review_process(2); // 2 for PvP mode
    }
    handle_save_record(2); // 2 for PvP mode
}

/**
 * @brief ??????????
 * @note ???????????????????????????
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
        printf("??????????????:\n");
        for (int i = 0; i < file_count; i++)
        {
            printf("%d. %s\n", i + 1, record_files[i]);
        }

        char prompt[150];
        sprintf(prompt, "??????????????(1-%d)????????0??????????????: ", file_count);
        int choice = get_integer_input(prompt, 0, file_count);

        if (choice > 0)
        {
            strcpy(filename, record_files[choice - 1]);
        }
        else
        {
            printf("???????????????: ");
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
        printf("???????????????????????????????: ");
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
            printf("????AI?????????????????\n");
        }
        else if (game_mode == 2)
        {
            printf("???????????????????????\n");
        }
        review_process(game_mode);
    }
}