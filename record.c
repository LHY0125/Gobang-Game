#include "record.h"
#include "game_mode.h"
#include "gobang.h"
#include "init_board.h"
#include "ui.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

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
 * - 评分环节调用calculate_final_score()函数
 */
void review_process(int game_mode)
{
    int review_choice = get_integer_input("是否要复盘本局比赛? (1-是, 0-否): ", 0, 1);
    
    // 如果评分尚未计算，则计算评分
    if (!scores_calculated)
    {
        calculate_game_scores();
    }
    else
    {
        // 评分已从文件中加载，直接使用
        printf("从记录文件中加载评分数据\n");
    }
    
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
            if (game_mode == GAME_MODE_AI)
            {
                // 人机对战
                printf("\n===== 五子棋人机对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER) ? "玩家" : "AI",
                       s.x + 1, s.y + 1);
            }
            else if (game_mode == GAME_MODE_PVP)
            {
                // 双人对战
                printf("\n===== 五子棋双人对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER1) ? "玩家1(黑棋)" : "玩家2(白棋)",
                       s.x + 1, s.y + 1);
            }
            else if (game_mode == GAME_MODE_NETWORK)
            {
                // 网络对战
                printf("\n===== 五子棋网络对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
                printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
                       i + 1, step_count,
                       (s.player == PLAYER1) ? "玩家1(黑棋)" : "玩家2(白棋)",
                       s.x + 1, s.y + 1);
            }

            // 打印当前复盘棋盘
            printf("  ");
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                printf("%2d", col + 1); // 列号
            }           
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
        
        // 显示胜负结果（直接使用文件中的信息）
        printf("\n===== 对局结果 =====");
        if (strcmp(winner_info, "玩家获胜") == 0)
        {
            printf("\n? 恭喜！玩家获胜！\n");
        }
        else if (strcmp(winner_info, "AI获胜") == 0)
        {
            printf("\n? AI获胜！\n");
        }
        else if (strcmp(winner_info, "玩家1获胜") == 0)
        {
            printf("\n? 恭喜！玩家1(黑棋)获胜！\n");
        }
        else if (strcmp(winner_info, "玩家2获胜") == 0)
        {
            printf("\n? 恭喜！玩家2(白棋)获胜！\n");
        }
        else
        {
            printf("\n?? 对局平局或未完成\n");
        }
        
        printf("\n复盘结束！按Enter查看评分...");
        getchar(); // 等待用户按键
    }

    // 显示评分结果
    display_game_scores(game_mode);

    getchar();
}

/**
 * @brief 计算游戏评分
 */
void calculate_game_scores()
{
    // 评估双方表现
    player1_final_score = 0;
    player2_final_score = 0;

    // 遍历所有步数，累积每一步的得分，后期步骤权重更高
    for (int i = 0; i < step_count; i++)
    {
        // 计算时间权重因子：步数越靠后，权重越大
        double time_weight = 1.0 + (double)i / step_count * TIME_WEIGHT_FACTOR; // 最后的步骤权重是开始步骤的(1+TIME_WEIGHT_FACTOR)倍
        
        if (steps[i].player == PLAYER || steps[i].player == PLAYER1)
        {
            player1_final_score += (int)(calculate_step_score(steps[i].x, steps[i].y, steps[i].player) * time_weight);
        }
        else
        {
            player2_final_score += (int)(calculate_step_score(steps[i].x, steps[i].y, steps[i].player) * time_weight);
        }
    }
    
    // 胜负加权：获胜方获得额外的评分奖励
    if (step_count > 0)
    {
        Step last_step = steps[step_count - 1];
        if (check_win(last_step.x, last_step.y, last_step.player))
        {
            // 获胜方获得额外奖励分数
            if (last_step.player == PLAYER || last_step.player == PLAYER1)
            {
                player1_final_score += WIN_BONUS; // 获胜奖励
            }
            else
            {
                player2_final_score += WIN_BONUS; // 获胜奖励
            }
        }
    }
    
    scores_calculated = 1; // 标记评分已计算
}

/**
 * @brief 显示游戏评分结果和MVP评选
 * @param game_mode 游戏模式(1-人机对战, 2-双人对战)
 */
void display_game_scores(int game_mode)
{
    printf("\n===== 对局评分 =====\n");
    double sum_score = (long double)player1_final_score + (long double)player2_final_score;

    if (sum_score > 0)
    {
        if (game_mode == GAME_MODE_AI)
        {
            printf("玩家得分: %d, 占比: %.2f%%\n",
                   player1_final_score, (double)player1_final_score * 100.0 / sum_score);
            printf("AI得分: %d, 占比: %.2f%%\n",
                   player2_final_score, (double)player2_final_score * 100.0 / sum_score);
        }
        else if (game_mode == GAME_MODE_PVP)
        {
            printf("玩家1(黑棋)得分: %d, 占比: %.2f%%\n",
                   player1_final_score, (double)player1_final_score * 100.0 / sum_score);
            printf("玩家2(白棋)得分: %d, 占比: %.2f%%\n",
                   player2_final_score, (double)player2_final_score * 100.0 / sum_score);
        }
        else if (game_mode == GAME_MODE_NETWORK)
        {
            printf("玩家1(黑棋)得分: %d, 占比: %.2f%%\n",
                   player1_final_score, (double)player1_final_score * 100.0 / sum_score);
            printf("玩家2(白棋)得分: %d, 占比: %.2f%%\n",
                   player2_final_score, (double)player2_final_score * 100.0 / sum_score);
        }
    }
    else
    {
        if (game_mode == GAME_MODE_AI)
        {
            printf("玩家得分: %d\n", player1_final_score);
            printf("AI得分: %d\n", player2_final_score);
        }
        else
        {
            printf("玩家1(黑棋)得分: %d\n", player1_final_score);
            printf("玩家2(白棋)得分: %d\n", player2_final_score);
        }
        printf("注: 双方得分均为0，无法计算占比\n");
    }

    // 评选MVP
    if (player1_final_score > player2_final_score)
    {
        printf("\nMVP: %s (领先 %d 分)\n", (game_mode == GAME_MODE_AI) ? "玩家" : "玩家1(黑棋)", player1_final_score - player2_final_score);
    }
    else if (player2_final_score > player1_final_score)
    {
        printf("\nMVP: %s (领先 %d 分)\n", (game_mode == GAME_MODE_AI) ? "AI" : "玩家2(白棋)", player2_final_score - player1_final_score);
    }
    else
    {
        printf("\n双方势均力敌！\n");
    }
}

/**
 * @brief 处理游戏结束后的记录保存
 * @return int 保存状态码(0-成功, 1-目录创建失败, 2-文件打开失败, 3-文件写入失败)
 */
void handle_save_record(int game_mode)
{
    printf("===== 游戏结束 =====\n");
    int save_choice = get_integer_input("是否保存游戏记录? (1-是, 0-否): ", 0, 1);

    if (save_choice == 1)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char filename[256];
        strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.csv", t);

        int save_status = save_game_to_file(filename, game_mode);
        switch (save_status)
        {
        case 0: // 成功
            printf("\n游戏记录已成功保存至: %s (CSV格式)\n", filename);
            printf("您可以使用以下命令进行复盘: .\\gobang.exe -l %s\n", filename);
            printf("CSV格式文件可以直接用Excel打开查看和分析\n");
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

    // 判断胜负结果
    strcpy(winner_info, "平局或未完成");
    if (step_count > 0)
    {
        Step last_step = steps[step_count - 1];
        if (check_win(last_step.x, last_step.y, last_step.player))
        {
            if (game_mode == GAME_MODE_AI)
            {
                // 人机对战
                if (last_step.player == PLAYER)
                {
                    strcpy(winner_info, "玩家获胜");
                }
                else
                {
                    strcpy(winner_info, "AI获胜");
                }
            }
            else if (game_mode == GAME_MODE_PVP)
            {
                // 双人对战
                if (last_step.player == PLAYER1)
                {
                    strcpy(winner_info, "玩家1获胜");
                }
                else
                {
                    strcpy(winner_info, "玩家2获胜");
                }
            }
            else if (game_mode == GAME_MODE_NETWORK)
            {
                // 网络对战
                if (last_step.player == PLAYER1)
                {
                    strcpy(winner_info, "玩家1获胜");
                }
                else
                {
                    strcpy(winner_info, "玩家2获胜");
                }
            }
        }
    }
    
    // 写入CSV文件头部
    if (fprintf(file, "游戏模式,棋盘大小,玩家1得分,玩家2得分,对局结果\n%d,%d,%d,%d,%s\n\n", game_mode, BOARD_SIZE, player1_final_score, player2_final_score, winner_info) < 0)
    {
        fclose(file);
        return 3; // 文件写入失败
    }
    
    // 写入CSV表头
    if (fprintf(file, "步数,玩家,行坐标,列坐标\n") < 0)
    {
        fclose(file);
        return 3; // 文件写入失败
    }

    // 写入所有落子步骤（CSV格式）
    for (int i = 0; i < step_count; i++)
    {
        if (fprintf(file, "%d,%d,%d,%d\n", i+1, steps[i].player, steps[i].x+1, steps[i].y+1) < 0)
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

    // 跳过CSV文件头部行
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), file) == NULL) // 跳过"游戏模式,棋盘大小"
    {
        fclose(file);
        return 0;
    }

    // 读取游戏模式、棋盘大小和评分结果
    int game_mode, size;
    
    // 尝试读取新格式（包含胜负信息）
    int read_count = fscanf(file, "%d,%d,%d,%d,%49s", &game_mode, &size, &player1_final_score, &player2_final_score, winner_info);
    
    if (read_count == 4)
    {
        // 旧格式文件，没有胜负信息
        strcpy(winner_info, "未知");
    }
    else if (read_count != 5)
    {
        // 文件格式错误
        fclose(file);
        return 0;
    }
    
    if (game_mode != GAME_MODE_AI && game_mode != GAME_MODE_PVP && game_mode != GAME_MODE_NETWORK)
    {
        fclose(file);
        return 0; // 无效的游戏模式
    }
    if (size < 5 || size > MAX_BOARD_SIZE)
    {
        fclose(file);
        return false;
    }
    
    // 设置评分已计算标志
    scores_calculated = 1;

    // 跳过空行和表头行
    fgets(buffer, sizeof(buffer), file); // 跳过换行
    fgets(buffer, sizeof(buffer), file); // 跳过空行
    fgets(buffer, sizeof(buffer), file); // 跳过"步数,玩家,行坐标,列坐标"

    // 初始化棋盘
    BOARD_SIZE = size;
    empty_board();

    // 读取所有落子步骤
    step_count = 0;
    int step_num; // 用于存储步数，但不使用
    while (fscanf(file, "%d,%d,%d,%d", &step_num, &steps[step_count].player, &steps[step_count].x, &steps[step_count].y) == 4)
    {
        // 将1-based坐标转换为0-based坐标
        steps[step_count].x--;
        steps[step_count].y--;
        step_count++;
    }

    fclose(file);
    return game_mode;
}