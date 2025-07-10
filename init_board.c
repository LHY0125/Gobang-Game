#include "init_board.h"
#include "gobang.h"
#include "game_mode.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 初始化棋盘为全空状态并重置步数计数器
 * 清空棋盘数组并将所有位置设为EMPTY，同时将step_count重置为0
 */
void empty_board()
{
    // 初始化棋盘状态为全空
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }
    step_count = 0; // 重置步数计数器
}

/**
 * @brief 打印当前棋盘状态
 * 以可读格式输出棋盘，包括行列号和棋子状态
 * 玩家棋子显示为'x'，AI棋子显示为'○'，空位显示为'·'
 */
void print_board()
{
    // 打印列号（1-BOARD_SIZE显示）
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d", i + 1);
        if (i + 1 == 9) // 处理列号9和10+的对齐
        {
            printf(" ");
        }
    }
    printf("\n");

    // 逐行打印棋盘内容
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d ", i + 1); // 打印行号（1-BOARD_SIZE）
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == PLAYER)
            {
                printf("x "); // 玩家棋子
            }
            else if (board[i][j] == AI)
            {
                printf("○ "); // AI棋子(使用○显示)
            }
            else
            {
                printf("· "); // 空位
            }
        }
        printf("\n"); // 每行结束换行
    }
}

/**
 * @brief 配置棋盘大小
 *
 * @param player1 玩家1
 * @param player2 玩家2
 */
void setup_board_size()
{
    printf("通常棋盘大小分为休闲棋盘(13X13)、标准棋盘(15X15)和特殊棋盘(19X19)\n");
    char prompt[100];
    sprintf(prompt, "请输入棋盘大小(5~%d)(默认为标准棋盘):\n", MAX_BOARD_SIZE);
    BOARD_SIZE = get_integer_input(prompt, 5, MAX_BOARD_SIZE);
}

/**
 * @brief Set the up game options object
 * 配置游戏选项，包括禁手规则、计时器和时间限制
 */
void setup_game_options()
{
    use_forbidden_moves = get_integer_input("是否启用禁手规则 (1-是, 0-否): ", 0, 1);

    use_timer = get_integer_input("是否启用计时器 (1-是, 0-否): ", 0, 1);
    if (use_timer)
    {
        time_limit = get_integer_input("请输入每回合的时间限制 (1~60分钟): ", 1, 60) * 60;
    }
}

/**
 * @brief 确定先手玩家
 *
 * @param player1
 * @param player2
 * @return int player1 or player2
 */
int determine_first_player(int player1, int player2)
{
    char prompt[100];
    sprintf(prompt, "请选择先手方 (1 for Player %d, 2 for Player %d): ", player1, player2);
    int first_player_choice = get_integer_input(prompt, 1, 2);
    if (first_player_choice == 1)
    {
        return player1;
    }
    else
    {
        return player2;
    }
}