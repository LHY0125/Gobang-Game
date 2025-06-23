#include "gobang.h"
#include <stdio.h>

/**
 * @brief 将指令复制到powershell
 * gcc 五子棋.c gobang.c -o output/五子棋.exe
 * gcc 为编译器，五子棋.c gobang.c 为源文件，output/为输出目录
 * @brief 将指令复制到powershell
 * .\output\五子棋.exe
 */

int main()
{
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
        printf("\n请输入落子坐标(行 列，1~%d):", BOARD_SIZE);
        scanf("%d %d", &x, &y);
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

    return 0;
}