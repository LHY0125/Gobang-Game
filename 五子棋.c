#include "gobang.h"
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief 将指令复制到powershell
 * gcc 五子棋.c gobang.c -o output/五子棋.exe
 * gcc 为编译器，五子棋.c gobang.c 为源文件，output/为输出目录
 * @brief 将指令复制到powershell
 * .\output\五子棋.exe
 */

int main(int argc, char *argv[])
{
#ifdef _WIN32
    system("chcp 65001 > nul"); // 设置控制台编码为UTF-8
    SetConsoleOutputCP(65001);  // 设置控制台输出编码
    SetConsoleCP(65001);        // 设置控制台输入编码
#endif

    // 检查是否要加载历史记录
    if (argc == 3 && strcmp(argv[1], "-l") == 0)
    {
        if (load_game_from_file(argv[2]))
        {
            printf("成功加载历史记录: %s\n", argv[2]);
            review_process();
            return 0;
        }
        else
        {
            printf("加载历史记录失败: %s\n", argv[2]);
            return 1;
        }
    }

    // 初始化阶段：获取棋盘尺寸
    printf("===== 五子棋人机对战 =====\n");
    printf("通常棋盘大小分为休闲棋盘(13X13)、标准棋盘(15X15)和特殊棋盘(19X19)\n");
    printf("请输入棋盘大小(5~%d)(默认为标准棋盘):\n", MAX_BOARD_SIZE);
    scanf("%d", &BOARD_SIZE);

    // 校验输入是否合法，不合法时使用默认值
    if (BOARD_SIZE < 5 || BOARD_SIZE > MAX_BOARD_SIZE)
    {
        BOARD_SIZE = 15;
        printf("输入无效，使用默认标准棋盘15X15\n");
    }

    // 添加AI难度选择
    int AI_DEPTH = 3;
    printf("请选择AI难度(1~5), 数字越大越强，注意数字越大AI思考时间越长！):");
    scanf("%d", &AI_DEPTH);
    if (AI_DEPTH < 1 || AI_DEPTH > 5)
    {
        AI_DEPTH = 3;
        printf("输入无效，使用默认难度3\n");
    }

    empty_board(); // 初始化棋盘
    printf("===== 五子棋人机对战(%dX%d棋盘, AI难度%d) =====", BOARD_SIZE, BOARD_SIZE, AI_DEPTH);
    print_board(); // 打印初始空棋盘

    // 游戏主循环
    while (1)
    {
        // 玩家回合
        int x, y;
        char input[10];
        printf("\n请输入落子坐标(行 列，1~%d)，或输入R/r悔棋:", BOARD_SIZE);
        scanf("%s", input);

        // 处理悔棋
        if (input[0] == 'r' || input[0] == 'R')
        {
            if (return_move())
            {
                printf("悔棋成功！\n");
                print_board();
            }
            else
            {
                printf("无法悔棋！\n");
            }
            continue;
        }

        // 处理正常落子
        sscanf(input, "%d", &x);
        scanf("%d", &y);
        // 转换用户输入的1-base坐标为0-base索引
        x--;
        y--;

        // 验证并执行玩家移动
        if (!player_move(x, y)) // 无效位置处理
        {
            printf("坐标无效！请重新输入。\n");
            continue; // 跳回循环开头重新输入
        }
        print_board(); // 更新后打印棋盘

        // 检查玩家是否获胜
        if (check_win(x, y, PLAYER))
        {
            printf("\n玩家获胜！\n");
            review_process(); // 展示复盘
            break;            // 退出游戏循环
        }

        // AI回合
        printf("\nAI思考中...\n");
        ai_move(AI_DEPTH); // AI计算最佳落子位置
        print_board();     // 展示AI落子后的棋盘

        // 检查AI是否获胜（通过最后一步）
        Step last_step = steps[step_count - 1]; // 获取最后一步
        if (check_win(last_step.x, last_step.y, AI))
        {
            printf("\nAI获胜！\n");
            review_process(); // 展示复盘
            break;            // 退出游戏循环
        }

        // 检查平局（棋盘已满）
        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            review_process(); // 展示复盘
            break;            // 退出游戏循环
        }
    }

    // 游戏结束，保存记录
    int save_result = 0;
    printf("===== 游戏结束 =====\n");
    printf("如果想保存记录，输入1");
    scanf("%d", &save_result);
    if (save_result)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char filename[256];
        strftime(filename, sizeof(filename), "records/%Y%m%d_%H%M%S.txt", t);

        int save_result = save_game_to_file(filename);
        switch (save_result)
        {
        case 0: // 成功
            printf("\n游戏记录已保存到: %s\n", filename);
            printf("可以使用以下命令复盘: .\\五子棋.exe -l %s\n", filename);
            break;
        case 1: // 目录创建失败
            printf("\n游戏记录保存失败: 无法创建records目录\n");
            printf("请检查是否有写入权限或磁盘空间是否充足\n");
            break;
        case 2: // 文件打开失败
            printf("\n游戏记录保存失败: 无法创建文件 %s\n", filename);
            printf("请检查是否有写入权限或路径是否有效\n");
            break;
        case 3: // 文件写入失败
            printf("\n游戏记录保存失败: 写入文件时出错\n");
            printf("请检查磁盘空间是否充足\n");
            break;
        default:
            printf("\n游戏记录保存失败: 未知错误\n");
        }
    }

    return 0;
}
