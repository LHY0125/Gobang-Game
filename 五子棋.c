#include "game_mode.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

/**
 * @brief 将指令复制到powershell
 * gcc 五子棋.c gobang.c game_mode.c -o output/五子棋.exe
 * gcc 为编译器，五子棋.c gobang.c game_mode.c 为源文件，output/为输出目录
 * @brief 将指令复制到powershell
 * .\output\五子棋.exe
 */

int main(int argc, char *argv[])
{
    // 设置控制台编码为UTF-8
#ifdef _WIN32
    system("chcp 65001 > nul"); // 设置控制台编码为UTF-8
    SetConsoleOutputCP(65001);  // 设置控制台输出编码
    SetConsoleCP(65001);        // 设置控制台输入编码
    _mkdir("records");
#endif

    // 选择模式
    printf("===== 五子棋游戏 =====\n");
    printf("1. AI模式\n");
    printf("2. 玩家比赛\n");
    printf("3. 复盘模式\n");
    int mode = get_integer_input("请输入模式(1/2/3): ", 1, 3);

    if (mode == 1)
    {
        run_ai_game();
    }
    else if (mode == 2)
    {
        run_pvp_game();
    }
    else if (mode == 3)
    {
        run_review_mode();
    }

    return 0;
}