/**
 * @file record.c
 * @brief 游戏复盘与记录源文件
 * @note 本文件定义了游戏复盘与记录相关的函数和数据结构。
 * 它负责管理游戏的历史记录、加载和保存游戏文件、计算游戏评分等功能。
 */

#include "record.h"
#include "gobang.h"
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
void calculate_game_scores();

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
 * @brief 将当前游戏记录保存到文件
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
        if (fprintf(file, "%d,%d,%d,%d\n", i + 1, steps[i].player, steps[i].x + 1, steps[i].y + 1) < 0)
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