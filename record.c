#include "record.h"
#include "game_mode.h"
#include "gobang.h"
#include "init_board.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/**
 * @brief 复盘游戏全过程并展示评分
 * @note 实现流程:
 * 1. 初始化临时复盘棋盘
 * 2. 按步数顺序逐步重现每个落子
 * 3. 每步显示:
 *    - 当前步数/总步数
 *    - 落子方(玩家/AI)
 *    - 落子位置(1-based坐标)
 *    - 当前棋盘状态
 * 4. 通过用户按Enter键控制步骤前进
 * 5. 复盘结束后自动进入评分环节:
 *    - 评估双方表现
 *    - 显示得分
 *    - 评选MVP
 * @note 技术细节:
 * - 使用独立临时棋盘避免影响主游戏状态
 * - 坐标显示转换为1-based方便用户理解
 * - 包含输入缓冲区清理防止意外输入
 * - 评分环节调用evaluate_performance()函数
 */
void review_process(int game_mode)
{
    int review_choice = get_integer_input("是否要复盘本局比赛? (1-是, 0-否): ", 0, 1);
    if (review_choice == 1)
    {
        printf("\n===== 复盘记录(总步数：%d) =====\n", step_count);
        // 清空输入缓冲区
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        // 创建临时复盘棋盘
        int temp_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
        memset(temp_board, EMPTY, sizeof(temp_board)); // 初始化为空棋盘

        // 逐步重现游戏过程
        for (int i = 0; i < step_count; i++)
        {
            Step s = steps[i];               // 获取当前步骤
            temp_board[s.x][s.y] = s.player; // 在临时棋盘上落子

            // 打印当前步骤信息
            // 根据游戏模式显示不同的标题和玩家信息
            if (game_mode == 1)
            {
                // 人机对战
                printf("\n===== 五子棋人机对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER) ? "玩家" : "AI",
                       s.x + 1, s.y + 1);
            }
            else
            {
                // 双人对战
                printf("\n===== 五子棋双人对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER1) ? "玩家1(黑棋)" : "玩家2(白棋)",
                       s.x + 1, s.y + 1);
            }

            // 打印当前复盘棋盘
            printf("  ");
            for (int col = 0; col < BOARD_SIZE; col++)
                printf("%2d", col + 1); // 列号
            printf("\n");

            for (int row = 0; row < BOARD_SIZE; row++)
            {
                printf("%2d ", row + 1); // 行号
                for (int col = 0; col < BOARD_SIZE; col++)
                {
                    if (temp_board[row][col] == PLAYER || temp_board[row][col] == PLAYER1)
                    {
                        printf("x ");
                    }
                    else if (temp_board[row][col] == AI || temp_board[row][col] == PLAYER2)
                    {
                        printf("○ ");
                    }
                    else
                    {
                        printf("· ");
                    }
                }
                printf("\n"); // 行结束换行
            }

            // 如果不是最后一步，等待用户按键继续
            if (i < step_count - 1)
            {
                printf("\n按Enter继续下一步...");
                while (getchar() != '\n')
                    ; // 等待回车
            }
        }
        printf("\n复盘结束！按Enter查看评分...");
        getchar(); // 等待用户按键
    }

    // 评估双方表现
    printf("\n===== 对局评分 =====\n");
    int player1_score = 0, player2_score = 0;

    // 遍历所有步数，累积每一步的得分
    for (int i = 0; i < step_count; i++)
    {
        if (steps[i].player == PLAYER || steps[i].player == PLAYER1)
        {
            player1_score += calculate_step_score(steps[i].x, steps[i].y, steps[i].player);
        }
        else
        {
            player2_score += calculate_step_score(steps[i].x, steps[i].y, steps[i].player);
        }
    }

    double sum_score = (long double)player1_score + (long double)player2_score;

    if (sum_score > 0)
    {
        if (game_mode == 1)
        {
            printf("玩家得分: %d, 占比: %.2f%%\n",
                   player1_score, (double)player1_score * 100.0 / sum_score);
            printf("AI得分: %d, 占比: %.2f%%\n",
                   player2_score, (double)player2_score * 100.0 / sum_score);
        }
        else
        {
            printf("玩家1(黑棋)得分: %d, 占比: %.2f%%\n",
                   player1_score, (double)player1_score * 100.0 / sum_score);
            printf("玩家2(白棋)得分: %d, 占比: %.2f%%\n",
                   player2_score, (double)player2_score * 100.0 / sum_score);
        }
    }
    else
    {
        if (game_mode == 1)
        {
            printf("玩家得分: %d\n", player1_score);
            printf("AI得分: %d\n", player2_score);
        }
        else
        {
            printf("玩家1(黑棋)得分: %d\n", player1_score);
            printf("玩家2(白棋)得分: %d\n", player2_score);
        }
        printf("注: 双方得分均为0，无法计算占比\n");
    }

    // 评选MVP
    if (player1_score > player2_score)
    {
        printf("\nMVP: %s (领先 %d 分)\n", (game_mode == 1) ? "玩家" : "玩家1(黑棋)", player1_score - player2_score);
    }
    else if (player2_score > player1_score)
    {
        printf("\nMVP: %s (领先 %d 分)\n", (game_mode == 1) ? "AI" : "玩家2(白棋)", player2_score - player1_score);
    }
    else
    {
        printf("\n双方势均力敌！\n");
    }

    getchar();
}

/**
 * @brief 处理游戏结束后的记录保存
 * @return int 保存状态码(0-成功, 1-目录创建失败, 2-文件打开失败, 3-文件写入失败)
 */
void handle_save_record(int game_mode)
{
    int save_choice = 0;
    printf("===== 游戏结束 =====\n");
    printf("是否保存游戏记录? (1-是, 0-否): ");
    scanf("%d", &save_choice);

    if (save_choice == 1)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char filename[256];
        strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.txt", t);

        int save_status = save_game_to_file(filename, game_mode);
        switch (save_status)
        {
        case 0: // 成功
            printf("\n游戏记录已成功保存至: %s\n", filename);
            printf("您可以使用以下命令进行复盘: .\\gobang.exe -l %s\n", filename);
            break;
        case 1: // 目录创建失败
            printf("\n游戏记录保存失败: 无法创建 'records' 目录。\n");
            printf("请检查程序是否具有足够的写入权限或磁盘空间是否充足。\n");
            break;
        case 2: // 文件打开失败
            printf("\n游戏记录保存失败: 无法在路径 '%s' 创建文件。\n", filename);
            printf("请检查路径是否有效以及程序是否具有写入权限。\n");
            break;
        case 3: // 文件写入失败
            printf("\n游戏记录保存失败: 写入文件时发生错误。\n");
            printf("请检查磁盘空间是否已满。\n");
            break;
        default:
            printf("\n游戏记录保存失败: 发生未知错误。\n");
            break;
        }
    }
}

/**
 * @brief 将当前游戏记录保存到文件
 * @param filename 要保存的文件名
 * @return int 错误码:
 *   0: 成功
 *   1: 目录创建失败
 *   2: 文件打开失败
 *   3: 文件写入失败
 */
int save_game_to_file(const char *filename, int game_mode)
{
    // 创建records目录(如果不存在)
    struct stat st = {0};
    if (stat("records", &st) == -1)
    {
        if (mkdir("records") != 0)
        {
            // 检查是否目录已存在(多线程情况下可能被其他线程创建)
            if (stat("records", &st) == -1)
            {
#ifdef _WIN32
                printf("错误：无法创建records目录\n");
                printf("可能原因：\n");
                printf("1. 没有写入权限 - 请尝试以管理员身份运行\n");
                printf("2. 防病毒软件阻止 - 请检查安全软件设置\n");
                printf("3. 路径无效 - 请检查工作目录\n");
#else
                perror("创建目录失败");
#endif
                return 1; // 目录创建失败
            }
        }
    }

    // 打开文件
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "w");
    if (!file)
    {
        return 2; // 文件打开失败
    }

    // 写入游戏模式和棋盘大小
    if (fprintf(file, "%d\n%d\n", game_mode, BOARD_SIZE) < 0)
    {
        fclose(file);
        return 3; // 文件写入失败
    }

    // 写入所有落子步骤
    for (int i = 0; i < step_count; i++)
    {
        if (fprintf(file, "%d %d %d\n", steps[i].player, steps[i].x, steps[i].y) < 0)
        {
            fclose(file);
            return 3; // 文件写入失败
        }
    }

    if (fclose(file) != 0)
    {
        return 3; // 文件关闭/写入失败
    }

    return 0; // 成功
}

/**
 * @brief 从文件加载游戏记录
 * @param filename 要加载的文件名
 * @return true 加载成功
 * @return false 加载失败
 */
int load_game_from_file(const char *filename)
{
    // 打开文件
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "r");
    if (!file)
    {
        return false;
    }

    // 读取游戏模式和棋盘大小
    int game_mode, size;
    if (fscanf(file, "%d", &game_mode) != 1 || (game_mode != 1 && game_mode != 2))
    {
        fclose(file);
        return 0; // 无效的游戏模式
    }
    if (fscanf(file, "%d", &size) != 1 || size < 5 || size > MAX_BOARD_SIZE)
    {
        fclose(file);
        return false;
    }

    // 初始化棋盘
    BOARD_SIZE = size;
    empty_board();

    // 读取所有落子步骤
    step_count = 0;
    while (fscanf(file, "%d %d %d", &steps[step_count].player, &steps[step_count].x, &steps[step_count].y) == 3)
    {
        step_count++;
    }

    fclose(file);
    return game_mode;
}