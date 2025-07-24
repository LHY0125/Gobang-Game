/**
 * @file main.c
 * @brief 五子棋游戏主函数文件
 * @note 本文件包含了游戏的主循环、模式选择和游戏初始化等功能
 * @brief 将以下指令复制到powershell
 * gcc -std=c17 -o gobang.exe *.c -lws2_32
   .\gobang.exe 
 * @detail gcc 为编译器，添加了-lws2_32链接Windows网络库
 * @detail 编译指令：gcc -std=c17 -o gobang.exe *.c -lws2_32
 * @detail 运行指令：.\gobang.exe
 */

#include "game_mode.h"
#include "ui.h"
#include "config.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

int main(int argc, char *argv[])
{
    // 设置控制台编码为UTF-8
#ifdef _WIN32
    system("chcp 65001 > nul"); // 设置控制台编码为UTF-8
    SetConsoleOutputCP(65001);  // 设置控制台输出编码
    SetConsoleCP(65001);        // 设置控制台输入编码
    _mkdir("records");
#endif

    // 加载游戏配置
    load_game_config();
    
    // 选择模式
    while(1)
    {
        clear_screen();
        display_main_menu();
        int mode = get_integer_input("请输入模式(0-7): ", 0, 7);

        switch (mode)
        {
            // 1. 人机对战
            case 1:
                run_ai_game();
                break;
            // 2. 玩家对战
            case 2:
                run_pvp_game();
                break;
            // 3. 网络对战
            case 3:
                run_network_game();
                break;
            // 4. 复盘模式
            case 4:
                run_review_mode();
                break;
            // 5. 配置管理
            case 5:
                config_management_menu();
                break;
            // 6. 游戏规则
            case 6:
                clear_screen();
                display_game_rules();
                pause_for_input("\n按任意键返回主菜单...");
                break;
            // 7. 关于游戏
            case 7:
                clear_screen();
                display_about();
                pause_for_input("\n按任意键返回主菜单...");
                break;
            // 0. 退出游戏
            case 0:
                save_game_config();
                printf("感谢使用五子棋游戏！\n");
                return 0;
            default:
                printf("无效的选择！\n");
                pause_for_input("按任意键继续...");
                break;
        }
    }

    return 0;
}