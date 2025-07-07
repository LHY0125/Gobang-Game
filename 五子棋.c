#include "game_mode.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

/**
 * @file gobang.h
 * @brief 五子棋游戏核心逻辑头文件
 * @details 定义了游戏的核心数据结构、全局变量、宏定义和函数原型。
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @date 2025-07-02
 * @version 4.0
 */

/**
 * @brief 将指令复制到powershell
 * gcc -o gobang.exe gobang.c ai.c game_mode.c init_board.c record.c 五子棋.c
 * gcc 为编译器，五子棋.c gobang.c game_mode.c 为源文件，output/为输出目录
 * @brief 将指令复制到powershell
 * .\gobang.exe
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
    while(1)
    {
        printf("===== 五子棋游戏 =====\n");
        printf("1. AI模式\n");
        printf("2. 玩家比赛\n");
        printf("3. 复盘模式\n");
        printf("4. 退出游戏\n");
        int mode = get_integer_input("请输入模式(1/2/3/4): ", 1, 4);

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
        else if (mode == 4)
        {
            printf("感谢使用五子棋游戏！\n");
            break;
        }
    }
    
    return 0;
}