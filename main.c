#include "game_mode.h"
#include "ui.h"
#include "config.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

/**
 * @brief 将指令复制到powershell
 * gcc -o gobang.exe main.c gobang.c game_mode.c ai.c record.c init_board.c ui.c config.c network.c globals.c -lws2_32
 * gcc 为编译器，添加了network.c网络模块，-lws2_32链接Windows网络库
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

    // 加载游戏配置
    load_game_config();
    
    // 选择模式
    while(1)
    {
        clear_screen();
        display_main_menu();
        int mode = get_integer_input("请输入模式(1-8): ", 1, 8);

        switch (mode)
        {
            case 1:
                run_ai_game();
                break;
            case 2:
                run_pvp_game();
                break;
            case 3:
                run_network_game();
                break;
            case 4:
                run_review_mode();
                break;
            case 5:
                config_management_menu();
                break;
            case 6:
                clear_screen();
                display_game_rules();
                pause_for_input("\n按任意键返回主菜单...");
                break;
            case 7:
                clear_screen();
                display_about();
                pause_for_input("\n按任意键返回主菜单...");
                break;
            case 8:
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