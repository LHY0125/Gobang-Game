/**
 * @file main.c
 * @brief 五子棋游戏主函数文件
 * @note 本文件包含了游戏的主循环、模式选择和游戏初始化等功能
 * @brief 将以下指令复制到powershell
 *
 * !图形化版本编译（需要IUP库）：
 * mingw32-make gui
   .\bin\gobang_gui.exe
 *
 * @note gcc 为编译器，添加了-lws2_32链接Windows网络库
 * @note IUP 的路径：libs\iup-3.31_Win64_dllw6_lib
 * @brief & "D:\Program Files (x86)\Inno Setup 6\iscc.exe" installer\\installer.iss
 */

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

    // 启动图形化界面
    run_gui_mode();
    return 0;
}