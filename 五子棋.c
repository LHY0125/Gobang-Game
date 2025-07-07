/**
 * @file 五子棋.c
 * @brief 五子棋游戏核心逻辑头文件
 * @details 游戏核心逻辑实现
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @date 2025-07-02
 * @version 4.0
 * @note
 * 1. 新增功能：
 *    - 增加了对禁手规则的支持，防止玩家进行无意义的走法。
 *    - 新增了游戏计时器功能，限制每回合的思考时间。
 * 2. 性能优化：
 *    - 优化了评估函数的性能，减少了不必要的计算。
 *    - 引入了 Alpha-Beta 剪枝算法，提高了 AI 搜索的效率。
 * 3. 用户界面改进：
 *    - 新增了命令行界面，提供更友好的交互体验。
 *    - 可以自定义棋盘大小，增加游戏的灵活性。
 * 4. 代码结构优化：
 *    - 将游戏逻辑和用户界面分离，提高代码的可读性和可维护性。
 *    - 优化了代码结构，提高了代码的可读性和可维护性。
 * 5. 异常处理：
 *    - 增加了输入错误的异常处理机制，确保游戏的稳定性。
 *    - 修复了一些已知的 bug，提高游戏的稳定性。
 * 6. 文档更新：
 *    - 完善了代码注释，提高了代码的可读性。
 *    - 更新了文档，包括功能描述、使用方法、注意事项等。
 * 7. 版本控制：
 *    - 使用 Git 进行版本控制，方便团队协作和代码管理。
 * 8. 测试：
 *    - 进行了全面的测试，确保游戏的稳定性和功能的正确性。
 * 9. 开源协议：
 *    - 选择了 MIT 开源协议，允许用户自由使用、修改和分发代码。
 * 10. 贡献者：
 *    - 刘航宇
 * 11. 联系信息：
 *    - 项目主页：[https://github.com/LHY0125/Gobang-Game]
 *    - 联系邮箱：[3364451258@qq.com][15236416560@163.com][lhy3364451258@outlook.com]
 */

#include "game_mode.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

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