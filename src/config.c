/**
 * @file config.c
 * @brief 五子棋游戏参数配置源文件
 * @note 本文件集中定义了五子棋游戏的所有参数配置，便于统一管理和修改
 */

#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 加载游戏配置
 */
void load_game_config()
{
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL)
    {
        // 配置文件不存在，使用默认配置
        printf("配置文件不存在，使用默认配置\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        // 去除换行符
        line[strcspn(line, "\n")] = 0;

        // 解析配置项
        if (strncmp(line, "BOARD_SIZE=", 11) == 0)
        {
            int size = atoi(line + 11);
            if (size >= MIN_BOARD_SIZE && size <= MAX_BOARD_SIZE)
            {
                BOARD_SIZE = size;
            }
        }
        else if (strncmp(line, "USE_FORBIDDEN_MOVES=", 20) == 0)
        {
            use_forbidden_moves = (atoi(line + 20) != 0);
        }
        else if (strncmp(line, "USE_TIMER=", 10) == 0)
        {
            use_timer = atoi(line + 10);
        }
        else if (strncmp(line, "TIME_LIMIT=", 11) == 0)
        {
            time_limit = atoi(line + 11);
        }
        else if (strncmp(line, "NETWORK_PORT=", 13) == 0)
        {
            int port = atoi(line + 13);
            if (port >= MIN_NETWORK_PORT && port <= MAX_NETWORK_PORT)
            {
                network_port = port;
            }
        }
        else if (strncmp(line, "NETWORK_TIMEOUT=", 16) == 0)
        {
            int timeout = atoi(line + 16);
            if (timeout > 0)
            {
                network_timeout = timeout;
            }
        }
        else if (strncmp(line, "AI_DIFFICULTY=", 14) == 0)
        {
            int difficulty = atoi(line + 14);
            if (difficulty >= 1 && difficulty <= 5)
            {
                ai_difficulty = difficulty;
                defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT + (ai_difficulty - 1) * 0.1;
            }
        }
    }

    fclose(file);
    printf("配置加载完成\n");
}

/**
 * @brief 保存游戏配置
 */
void save_game_config()
{
    FILE *file = fopen(CONFIG_FILE, "w");
    if (file == NULL)
    {
        printf("无法保存配置文件\n");
        return;
    }

    fprintf(file, "# 五子棋游戏配置文件\n");
    fprintf(file, "# 棋盘大小 (范围: %d-%d)\n", MIN_BOARD_SIZE, MAX_BOARD_SIZE);
    fprintf(file, "BOARD_SIZE=%d\n", BOARD_SIZE);
    fprintf(file, "\n# 禁手规则 (0=关闭, 1=开启)\n");
    fprintf(file, "USE_FORBIDDEN_MOVES=%d\n", use_forbidden_moves ? 1 : 0);
    fprintf(file, "\n# 计时器 (0=关闭, 1=开启)\n");
    fprintf(file, "USE_TIMER=%d\n", use_timer);
    fprintf(file, "\n# 时间限制 (分钟)\n");
    fprintf(file, "TIME_LIMIT=%d\n", time_limit);
    fprintf(file, "\n# 网络端口 (范围: %d-%d)\n", MIN_NETWORK_PORT, MAX_NETWORK_PORT);
    fprintf(file, "NETWORK_PORT=%d\n", network_port);
    fprintf(file, "\n# 网络超时时间 (毫秒)\n");
    fprintf(file, "NETWORK_TIMEOUT=%d\n", network_timeout);

    fprintf(file, "\n# AI难度 (1-5)\n");
    fprintf(file, "AI_DIFFICULTY=%d\n", ai_difficulty);

    fclose(file);
    printf("配置保存完成\n");
}

/**
 * @brief 重置为默认配置
 */
void reset_to_default_config()
{
    BOARD_SIZE = DEFAULT_BOARD_SIZE;
    use_forbidden_moves = DEFAULT_USE_FORBIDDEN_MOVES;
    use_timer = DEFAULT_USE_TIMER;
    time_limit = DEFAULT_TIME_LIMIT;
    network_port = DEFAULT_NETWORK_PORT;
    network_timeout = NETWORK_TIMEOUT_MS;
    ai_difficulty = 3; // 默认AI难度
    defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT + (ai_difficulty - 1) * 0.1;

    printf("已重置为默认配置\n");
}