#include "ui.h"
#include "gobang.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif

/**
 * @brief 显示游戏主菜单
 */
void display_main_menu()
{
    printf("===== 五子棋游戏 =====\n");
    printf("1. AI模式\n");
    printf("2. 玩家比赛\n");
    printf("3. 网络对战\n");
    printf("4. 复盘模式\n");
    printf("5. 游戏设置\n");
    printf("6. 游戏规则\n");
    printf("7. 关于游戏\n");
    printf("0. 退出游戏\n");
    printf("=====================\n");
}

/**
 * @brief 显示棋盘
 */
void display_board()
{
    printf("\n  ");
    // 打印列号
    for (int j = 0; j < BOARD_SIZE; j++)
    {
        printf("%2d", j);
    }
    printf("\n");
    
    // 打印棋盘内容
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d", i); // 打印行号
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == EMPTY)
            {
                printf(" ·"); // 空位用点表示
            }
            else if (board[i][j] == PLAYER || board[i][j] == PLAYER1)
            {
                printf(" ●"); // 玩家1用实心圆表示
            }
            else
            {
                printf(" ○"); // 玩家2/AI用空心圆表示
            }
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * @brief 显示游戏状态信息
 * @param current_player 当前玩家
 * @param step_count 当前步数
 */
void display_game_status(int current_player, int step_count)
{
    printf("当前步数: %d\n", step_count);
    if (current_player == PLAYER || current_player == PLAYER1)
    {
        printf("当前玩家: ●\n");
    }
    else
    {
        printf("当前玩家: ○\n");
    }
}

/**
 * @brief 显示获胜信息
 * @param winner 获胜者
 */
void display_winner(int winner)
{
    printf("\n游戏结束！\n");
    if (winner == PLAYER)
    {
        printf("玩家获胜！\n");
    }
    else if (winner == AI)
    {
        printf("AI获胜！\n");
    }
    else if (winner == PLAYER1)
    {
        printf("玩家1获胜！\n");
    }
    else if (winner == PLAYER2)
    {
        printf("玩家2获胜！\n");
    }
    else
    {
        printf("平局！\n");
    }
}

/**
 * @brief 显示游戏设置菜单
 */
void display_settings_menu()
{
    printf("\n===== 游戏设置 =====\n");
    printf("1. 棋盘大小设置\n");
    printf("2. 禁手规则设置\n");
    printf("3. 计时器设置\n");
    printf("4. 网络配置设置\n");
    printf("5. AI难度设置\n");
    printf("0. 返回主菜单\n");
    printf("==================\n");
}

/**
 * @brief 清屏函数
 */
void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief 暂停等待用户输入
 * @param prompt 提示信息
 */
void pause_for_input(const char* prompt)
{
    printf("%s", prompt);
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
}

/**
 * @brief 显示游戏规则
 */
void display_game_rules()
{
    printf("\n===== 五子棋游戏规则 =====\n");
    printf("1. 🎮 游戏目标：\n");
    printf("   在棋盘上连成五个同色棋子（横、竖、斜均可）\n\n");
    printf("2. 🔄 游戏流程：\n");
    printf("   - ⚫ 黑棋先行，双方轮流落子\n");
    printf("   - 📍 输入坐标格式：行号 列号（如：7 7）\n");
    printf("   - ✨ 棋子落在棋盘交叉点上\n\n");
    printf("3. 🏆 胜负判定：\n");
    printf("   - 🎉 率先连成五子者获胜\n");
    printf("   - 🤝 棋盘下满无人获胜则为平局\n\n");
    printf("4. 🚫 禁手规则（可选）：\n");
    printf("   - ❌ 三三禁手：同时形成两个活三\n");
    printf("   - ❌ 四四禁手：同时形成两个冲四\n");
    printf("   - ❌ 长连禁手：连成六子或以上\n\n");
    printf("5. 🛠️ 特殊功能：\n");
    printf("   - ↩️ 悔棋：输入 'R' 或 'r' 可悔棋\n");
    printf("   - 💾 保存：游戏结束后可保存对局记录\n");
    printf("   - 📖 复盘：可加载历史对局进行复盘\n");
    printf("============================\n");
}

/**
 * @brief 显示关于信息
 */
void display_about()
{
    printf("\n===== 关于五子棋游戏 =====\n");
    printf("🎮 游戏名称：五子棋人机对战\n");
    printf("📦 版本：7.0\n");
    printf("👨‍💻 开发者：刘航宇\n");
    printf("📧 联系邮箱：3364451258@qq.com\n");
    printf("🌐 项目主页：https://github.com/LHY0125/Gobang-Game\n\n");
    printf("✨ 主要特性：\n");
    printf("   🤖 智能AI对战（支持多种难度）\n");
    printf("   👥 双人对战模式\n");
    printf("   🌐 网络对战（局域网/互联网）\n");
    printf("   📝 对局记录与复盘\n");
    printf("   🚫 禁手规则支持\n");
    printf("   ⏱️ 计时器功能\n");
    printf("   📏 自定义棋盘大小\n");
    printf("   📊 评分系统\n\n");
    printf("🙏 感谢使用！\n");
    printf("========================\n");
}