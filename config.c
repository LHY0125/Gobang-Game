#include "config.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 配置文件路径
#define CONFIG_FILE "gobang_config.ini"

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
        else if (strncmp(line, "AI_DIFFICULTY=", 14) == 0)
        {
            int difficulty = atoi(line + 14);
            if (difficulty >= 1 && difficulty <= 5)
            {
                // 根据难度设置AI搜索深度
                // 这里可以添加AI难度相关的配置
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
    
    printf("已重置为默认配置\n");
}

/**
 * @brief 显示当前配置
 */
void display_current_config()
{
    printf("\n===== 当前游戏配置 =====\n");
    printf("棋盘大小: %d x %d\n", BOARD_SIZE, BOARD_SIZE);
    printf("禁手规则: %s\n", use_forbidden_moves ? "开启" : "关闭");
    printf("计时器: %s\n", use_timer ? "开启" : "关闭");
    if (use_timer)
    {
        printf("时间限制: %d 分钟\n", time_limit / 60);
    }
    printf("=====================\n");
}

/**
 * @brief 配置棋盘大小
 */
void config_board_size()
{
    printf("\n当前棋盘大小: %d x %d\n", BOARD_SIZE, BOARD_SIZE);
    printf("请输入新的棋盘大小 (%d-%d): ", MIN_BOARD_SIZE, MAX_BOARD_SIZE);
    
    int new_size;
    if (scanf("%d", &new_size) == 1)
    {
        if (new_size >= MIN_BOARD_SIZE && new_size <= MAX_BOARD_SIZE)
        {
            BOARD_SIZE = new_size;
            printf("棋盘大小已设置为: %d x %d\n", BOARD_SIZE, BOARD_SIZE);
        }
        else
        {
            printf("无效的棋盘大小！\n");
        }
    }
    else
    {
        printf("输入格式错误！\n");
        // 清除输入缓冲区
        while (getchar() != '\n');
    }
}

/**
 * @brief 配置禁手规则
 */
void config_forbidden_moves()
{
    printf("\n当前禁手规则: %s\n", use_forbidden_moves ? "开启" : "关闭");
    printf("是否启用禁手规则？(1=开启, 0=关闭): ");
    
    int choice;
    if (scanf("%d", &choice) == 1)
    {
        use_forbidden_moves = (choice != 0);
        printf("禁手规则已%s\n", use_forbidden_moves ? "开启" : "关闭");
    }
    else
    {
        printf("输入格式错误！\n");
        while (getchar() != '\n');
    }
}

/**
 * @brief 配置计时器
 */
void config_timer()
{
    printf("\n当前计时器: %s\n", use_timer ? "开启" : "关闭");
    printf("是否启用计时器？(1=开启, 0=关闭): ");
    
    int choice;
    if (scanf("%d", &choice) == 1)
    {
        use_timer = choice;
        if (use_timer)
        {
            printf("请输入时间限制(分钟): ");
            int new_limit;
            if (scanf("%d", &new_limit) == 1 && new_limit > 0)
             {
                 time_limit = new_limit * 60;  // 转换为秒数存储
                 printf("计时器已开启，时间限制: %d 分钟\n", time_limit / 60);
            }
            else
            {
                printf("无效的时间限制！\n");
                while (getchar() != '\n');
            }
        }
        else
        {
            printf("计时器已关闭\n");
        }
    }
    else
    {
        printf("输入格式错误！\n");
        while (getchar() != '\n');
    }
}

/**
 * @brief 配置管理主菜单
 */
void config_management_menu()
{
    int choice;
    
    while (1)
    {
        clear_screen();
        display_settings_menu();
        display_current_config();
        
        printf("请选择操作: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("输入格式错误！\n");
            while (getchar() != '\n');
            pause_for_input("按任意键继续...");
            continue;
        }
        
        switch (choice)
        {
            case 1:
                config_board_size();
                break;
            case 2:
                config_forbidden_moves();
                break;
            case 3:
                config_timer();
                break;
            case 4:
                printf("AI难度设置功能开发中...\n");
                break;
            case 5:
                save_game_config();
                return;
            default:
                printf("无效的选择！\n");
                break;
        }
        
        pause_for_input("按任意键继续...");
    }
}