/**
 * @file main.c
 * @brief 五子棋游戏主函数文件
 * @note 本文件包含了游戏的主循环、模式选择和游戏初始化等功能
 * @brief 将以下指令复制到powershell
 *
 * !控制台版本编译：
 * gcc -std=c17 -o gobang_console.exe *.c -lws2_32
   .\gobang_console.exe
 *
 * !图形化版本编译（需要SDL3）：
 * gcc -std=c17 -o gobang_gui.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
   copy "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\bin\SDL3.dll" .
   .\gobang_gui.exe
 *
 * @detail gcc 为编译器，添加了-lws2_32链接Windows网络库
 * @detail SDL3 的路径：D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32
 * @brief & "D:\Program Files (x86)\NSIS\makensis.exe" "installer\\installer.nsi"
 * @brief & "D:\Program Files (x86)\Inno Setup 6\iscc.exe" installer\\setup.iss
 */

#include "game_mode.h"
#include "ui.h"
#include "gui.h"
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
    while (1)
    {
        clear_screen();
        display_main_menu();
        int mode = get_integer_input("请输入模式(0-8): ", 0, 8);

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
        // 8. 图形化界面
        case 8:
            if (init_gui() == 0)
            {
                printf("启动图形化界面...\n");
                printf("图形化界面已启动，窗口应该可见\n");
                printf("如果看不到窗口，请检查任务栏或按Alt+Tab切换\n");
                while (gui_running && handle_events())
                {
                    render_game();
                    SDL_Delay(16); // 约60FPS
                }
                printf("退出图形化界面\n");
                cleanup_gui();
            }
            else
            {
                printf("图形化界面启动失败！请检查SDL3库是否正确安装。\n");
                pause_for_input("按任意键返回主菜单...");
            }
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