/**
 * @file game_mode.c
 * @brief 五子棋游戏框架源文件
 * @note 本文件定义了五子棋游戏的四种主要模式：
 * 1. AI对战模式
 * 2. 双人对战模式
 * 3. 网络对战模式
 * 4. 复盘模式
 */

#include "game_mode.h"
#include "init_board.h"
#include "gobang.h"
#include "ai.h"
#include "record.h"
#include "config.h"
#include "network.h"
#include "ui.h"
#include "gui.h"
#include "globals.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

// 全局变量现在在globals.c中定义
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <conio.h>
#endif

/**
 * @brief 从用户获取整数输入
 *
 * @param prompt 提示信息
 * @param min 最小值
 * @param max 最大值
 * @return int 用户输入的整数
 */
int get_integer_input(const char *prompt, int min, int max)
{
    int value;
    int result;
    char ch;

    while (1)
    {
        printf("%s", prompt);
        result = scanf("%d", &value);

        if (result == 1 && value >= min && value <= max)
        {
            // 清除输入缓冲区中剩余的字符
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            return value;
        }
        else
        {
            // 清除无效输入
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("输入无效，请输入一个介于 %d 和 %d 之间的整数。\n", min, max);
        }
    }
}

/**
 * @brief 处理玩家回合
 *
 * @param current_player
 * @return true
 * @return false
 */
bool parse_player_input(int *x, int *y)
{
    char input[10];

    while (1)
    {
        if (_kbhit())
        {
            scanf("%s", input);
            break;
        }
        Sleep(100); // 短暂延迟以防止CPU占用过高
    }

    if (sscanf(input, "%d", x) == 1)
    {
        // 成功解析第一个数字，现在解析第二个
        if (scanf("%d", y) != 1)
        {
            // 如果第二个数字不可用，则检查特殊命令
            if (*x == INPUT_UNDO)
            {
                int steps_to_undo;
                steps_to_undo = get_integer_input("请输入要悔棋的步数(双方各退步数相同): ", 1, step_count / 2);
                if (return_move(steps_to_undo * 2))
                {
                    printf("成功悔棋，双方各退 %d 步！\n", steps_to_undo);
                    print_board();
                }
                else
                {
                    printf("无法悔棋！\n");
                }
                return 0; // 特殊命令已处理
            }
            else if (*x == INPUT_SAVE)
            {
                // ... 处理保存 ...
                return false; // 特殊命令
            }
            else if (*x == INPUT_EXIT)
            {
                // ... 处理退出 ...
                return false; // 特殊命令
            }
            printf("无效输入，请输入两个数字坐标。\n");
            while (getchar() != '\n')
                ;
            return 0; // 无效输入
        }
    }
    else
    {
        // sscanf失败，检查特殊命令
        if (input[0] == 'r' || input[0] == 'R')
        {
            int steps_to_undo;
            steps_to_undo = get_integer_input("请输入要悔棋的步数(双方各退步数相同): ", 1, step_count / 2);
            if (return_move(steps_to_undo * 2))
            {
                printf("成功悔棋，双方各退 %d 步！\n", steps_to_undo);
                print_board();
            }
            else
            {
                printf("无法悔棋！\n");
            }
            return false; // 特殊命令
        }
        else if (input[0] == 's' || input[0] == 'S')
        {
            *x = INPUT_SURRENDER;
            int confirm = get_integer_input("确认认输？(1:是/0:否): ", 0, 1);
            if (confirm)
            {
                printf("玩家选择认输！\n");
                return 1; // 正常回合完成 // 返回认输命令
            }
            else
            {
                printf("取消认输！\n");
                return false; // 取消认输
            }
        }
        printf("无效输入，请输入数字坐标、'r'悔棋或's'认输。\n");
        return 0; // 无效输入
    }
    return 1; // 有效坐标
}

/**
 * @brief 解析网络对战模式下的玩家输入
 * @param x 行坐标指针
 * @param y 列坐标指针
 * @return true 有效坐标输入
 * @return false 特殊命令或无效输入
 */
bool parse_network_player_input(int *x, int *y)
{
    char input[10];

    while (1)
    {
        if (_kbhit())
        {
            scanf("%s", input);
            break;
        }
        Sleep(100); // 短暂延迟以防止CPU占用过高
    }

    if (sscanf(input, "%d", x) == 1)
    {
        // 成功解析第一个数字，现在解析第二个
        if (scanf("%d", y) != 1)
        {
            printf("无效输入，请输入两个数字坐标。\n");
            while (getchar() != '\n')
                ;
            return false; // 无效输入
        }
    }
    else
    {
        // sscanf失败，检查特殊命令
        if (input[0] == 'r' || input[0] == 'R')
        {
            int steps_to_undo;
            steps_to_undo = get_integer_input("请输入要悔棋的步数(双方各退步数相同): ", 1, step_count / 2);
            
            printf("发送悔棋请求给对方...\n");
            if (send_undo_request(steps_to_undo))
            {
                printf("悔棋请求已发送，等待对方回应...\n");
                
                // 等待对方回应
                NetworkMessage msg;
                time_t start_time = time(NULL);
                
                while (difftime(time(NULL), start_time) < 30) // 30秒超时
                {
                    if (receive_network_message(&msg, 1000))
                    {
                        if (msg.type == MSG_UNDO_RESPONSE && msg.x == steps_to_undo)
                        {
                            if (msg.y == 1) // 对方同意
                            {
                                if (return_move(steps_to_undo * 2))
                                {
                                    printf("对方同意悔棋，双方各退 %d 步！\n", steps_to_undo);
                                    print_board();
                                    return 2; // 悔棋成功，需要重新开始回合
                                }
                                else
                                {
                                    printf("悔棋失败！\n");
                                    return 0; // 悔棋失败，继续当前回合
                                }
                            }
                            else // 对方拒绝
                            {
                                printf("对方拒绝了悔棋请求。\n");
                                return 0; // 悔棋被拒绝，继续当前回合
                            }
                        }
                    }
                    
                    if (!is_network_connected())
                    {
                        printf("网络连接断开\n");
                        return 0;
                    }
                }
                
                printf("悔棋请求超时，对方未回应。\n");
            }
            else
            {
                printf("发送悔棋请求失败！\n");
            }
            return 0; // 特殊命令已处理
        }
        else if (input[0] == 's' || input[0] == 'S')
        {
            *x = INPUT_SURRENDER;
            int confirm = get_integer_input("确认认输？(1:是/0:否): ", 0, 1);
            if (confirm)
            {
                printf("你选择认输！\n");
                *x = INPUT_SURRENDER;
                return -1; // 返回认输命令
            }
            else
            {
                printf("取消认输！\n");
                return 0; // 取消认输
            }
        }
        printf("无效输入，请输入数字坐标、'r'悔棋或's'认输。\n");
        return false; // 无效输入
    }
    return true; // 有效坐标
}

/**
 * @brief 处理玩家回合
 * @param current_player 当前玩家
 * @return true
 */
bool handle_player_turn(int current_player)
{
    int x, y;
    time_t start_time, end_time;
    if (use_timer)
    {
        time(&start_time);
    }

    while (1)
    {
        printf("\n玩家%d, 请输入落子坐标(行 列，1~%d)，或输入R/r悔棋，S/s认输:", current_player, BOARD_SIZE);

        bool input_received = false;
        while (!input_received)
        {
            if (use_timer)
            {
                time(&end_time);
                if (difftime(end_time, start_time) > time_limit)
                {
                    printf("\n玩家%d超时, 对方获胜！\n", current_player);
                    return false; // Timeout
                }
            }

            if (parse_player_input(&x, &y))
            {
                if (x == INPUT_SURRENDER)
                {
                    printf("\n玩家%d选择认输，对方获胜！\n", current_player);
                    return false; // 游戏结束，认输
                }
                input_received = true;
            }
            else
            {
                // 已处理特殊命令或无效输入，继续循环
                return true;
            }
        }

        x--;
        y--;

        if (player_move(x, y, current_player))
        {
            break; // 成功落子，跳出循环
        }
        else
        {
            printf("坐标无效！请重新输入。\n");
            // 继续循环，重新输入坐标
        }
    }
    print_board();

    if (check_win(x, y, current_player))
    {
        printf("\n玩家%d获胜！\n", current_player);
        return false; // 游戏结束
    }

    return true; // 成功落子
}

/**
 * @brief 运行AI游戏
 * @note 从文件中加载历史记录并进行复盘
 * @param AI_DEPTH AI的搜索深度
 */
void run_ai_game()
{
    // 重置评分计算标志，确保每局游戏都会重新计算评分
    scores_calculated = 0;

    // AI对战模式
    int AI_DEPTH = DEFAULT_AI_DEPTH;
    AI_DEPTH = get_integer_input("请选择AI难度(1~5), 数字越大越强，注意数字越大AI思考时间越长！):", 1, 5);

    /**
     * @brief AI的防守系数，系数越大越倾向于防守
     * @note 1~1.5
     *       2~1.6
     *       3~1.7
     *       4~1.8
     *       5~1.9
     */
    defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT + (AI_DEPTH - 1) * 0.1;

    empty_board();
    int current_player = determine_first_player(PLAYER, AI);
    print_board();

    while (1)
    {
        if (current_player == PLAYER)
        {
            int old_step_count = step_count;
            if (!handle_player_turn(current_player))
            {
                break; // 游戏结束或超时
            }
            if (step_count > old_step_count)
            {
                // 检查玩家是否获胜
                Step last_step = steps[step_count - 1];
                if (check_win(last_step.x, last_step.y, PLAYER))
                {
                    printf("\n玩家获胜！\n");
                    break;
                }
                current_player = AI;
            }
        }
        else
        {
            printf("\nAI思考中...\n");
            time_t start_time, end_time;
            if (use_timer)
            {
                time(&start_time);
            }

            ai_move(AI_DEPTH);

            if (use_timer)
            {
                time(&end_time);
                if (difftime(end_time, start_time) > time_limit)
                {
                    printf("\nAI超时, 玩家获胜！\n");
                    break;
                }
            }
            print_board();
            Step last_step = steps[step_count - 1];
            if (check_win(last_step.x, last_step.y, AI))
            {
                printf("\nAI获胜！\n");
                break;
            }
            current_player = PLAYER;
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            break;
        }
    }
    printf("===== 游戏结束 =====\n");
    review_process(GAME_MODE_AI);     // AI对战模式
    handle_save_record(GAME_MODE_AI); // AI对战模式
}

/**
 * @brief 运行双人对战模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_pvp_game()
{
    // 重置评分计算标志，确保每局游戏都会重新计算评分
    scores_calculated = 0;

    // 双人对战模式
    empty_board();
    int current_player = determine_first_player(PLAYER1, PLAYER2);
    print_board();

    while (1)
    {
        int old_step_count = step_count;
        if (!handle_player_turn(current_player))
        {
            break; // 游戏结束或超时
        }

        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            break;
        }

        if (step_count > old_step_count)
        {
            current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
        }
    }
    printf("===== 游戏结束 =====\n");
    review_process(GAME_MODE_PVP);     // 双人对战模式
    handle_save_record(GAME_MODE_PVP); // 双人对战模式
}

/**
 * @brief 运行复盘模式
 * @note 从文件中加载历史记录并进行复盘
 */
void run_review_mode()
{
    char filename[100];
    char record_files[100][100];
    int file_count = 0;

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("records\\*", &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcpy(record_files[file_count++], ffd.cFileName);
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
    }
#endif

    if (file_count > 0)
    {
        printf("发现以下复盘文件:\n");
        for (int i = 0; i < file_count; i++)
        {
            printf("%d. %s\n", i + 1, record_files[i]);
        }

        char prompt[150];
        sprintf(prompt, "请输入复盘文件编号(1-%d)，或输入0以手动输入文件名: ", file_count);
        int choice = get_integer_input(prompt, 0, file_count);

        if (choice > 0)
        {
            strcpy(filename, record_files[choice - 1]);
        }
        else
        {
            printf("请输入完整文件名: ");
            scanf("%s", filename);
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;

            int possible_choice = atoi(filename);
            if (possible_choice > 0 && possible_choice <= file_count)
            {
                strcpy(filename, record_files[possible_choice - 1]);
            }
        }
    }
    else
    {
        printf("未找到任何复盘文件，请输入复盘文件地址: ");
        scanf("%s", filename);
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    int game_mode = load_game_from_file(filename);
    if (game_mode != 0)
    {
        if (game_mode == 1)
        {
            printf("加载AI对战模式复盘文件成功！\n");
        }
        else if (game_mode == 2)
        {
            printf("加载双人对战模式复盘文件成功！\n");
        }
        review_process(game_mode);
    }
    else
    {
        printf("加载复盘文件失败！可能是旧版本文件格式或文件损坏\n");
    }
}

/**
 * @brief 网络对战模式
 */
void run_network_game()
{
    // 重置评分计算标志
    scores_calculated = 0;
    
    // 初始化网络模块
    if (!init_network())
    {
        printf("网络初始化失败！\n");
        pause_for_input("按任意键返回主菜单...");
        return;
    }
    
    printf("=== 网络对战模式 ===\n");
    printf("1. 创建房间（作为主机）\n");
    printf("2. 加入房间（连接到主机）\n");
    
    int choice = get_integer_input("请选择模式(1-2): ", 1, 2);
    
    bool connection_success = false;
    
    if (choice == 1)
    {
        // 服务器模式
        int port = get_integer_input("请输入监听端口(默认8888): ", MIN_NETWORK_PORT, MAX_NETWORK_PORT);
        if (port == 0) port = network_port;
        
        printf("\n正在创建房间...\n");
        connection_success = create_server(port);
    } 
    else
    {
        // 客户端模式
        char ip[MAX_IP_LENGTH];
        
        // 循环直到输入有效的IP地址或用户选择退出
        while (1)
        {
            printf("请输入服务器IP地址 (输入'exit'退出): ");
            if (scanf("%s", ip) != 1)
            {
                printf("输入错误，请重新输入。\n");
                // 清除输入缓冲区
                while (getchar() != '\n');
                continue;
            }
            
            // 检查是否要退出
            if (strcmp(ip, "exit") == 0 || strcmp(ip, "EXIT") == 0)
            {
                printf("取消连接，返回主菜单。\n");
                cleanup_network();
                pause_for_input("按任意键返回主菜单...");
                return;
            }
            
            // 简单的IP地址格式验证
            if (strlen(ip) < 7 || strlen(ip) > 15)
            {
                printf("IP地址格式错误！请输入有效的IP地址（如：192.168.1.100）\n");
                continue;
            }
            
            // 检查IP地址是否包含有效字符
            bool valid_ip = true;
            for (int i = 0; i < strlen(ip); i++)
            {
                if (!(isdigit(ip[i]) || ip[i] == '.'))
                {
                    valid_ip = false;
                    break;
                }
            }
            
            if (!valid_ip)
            {
                printf("IP地址格式错误！只能包含数字和点号。\n");
                continue;
            }
            
            // 检查点号数量
            int dot_count = 0;
            for (int i = 0; i < strlen(ip); i++)
            {
                if (ip[i] == '.') dot_count++;
            }
            
            if (dot_count != 3)
            {
                printf("IP地址格式错误！应包含3个点号（如：192.168.1.100）\n");
                continue;
            }
            
            printf("输入的IP地址: %s\n", ip);
            int confirm = get_integer_input("确认连接到此IP？(1:是/0:否，重新输入): ", 0, 1);
            if (confirm)
            {
                break; // 确认IP地址，跳出循环
            }
            // 如果选择否，继续循环重新输入
        }
        
        int port = get_integer_input("请输入服务器端口(默认8888): ", MIN_NETWORK_PORT, MAX_NETWORK_PORT);
        if (port == 0) port = network_port;
        
        printf("\n正在连接到服务器 %s:%d...\n", ip, port);
        connection_success = connect_to_server(ip, port);
    }
    
    if (!connection_success)
    {
        printf("网络连接失败！\n");
        cleanup_network();
        pause_for_input("按任意键返回主菜单...");
        return;
    }
    
    printf("\n网络连接成功！游戏即将开始...\n");
    printf("你是玩家%d，%s先手\n", 
           network_state.local_player_id,
           network_state.local_player_id == PLAYER1 ? "你" : "对方");
    
    // 开始网络游戏
    empty_board();
    print_board();
    
    if (network_game_loop())
    {
        printf("===== 游戏结束 =====\n");
        review_process(GAME_MODE_NETWORK);     // 网络对战模式的复盘
        handle_save_record(GAME_MODE_NETWORK); // 保存为网络对战模式记录
    }
    else
    {
        printf("游戏因网络错误而结束\n");
    }
    
    // 清理网络连接
    disconnect_network();
    pause_for_input("按任意键返回主菜单...");
}

/**
 * @brief 显示游戏规则
 */
void show_game_rules()
{
    clear_screen();
    display_game_rules();
    pause_for_input("\n按任意键返回主菜单...");
}

/**
 * @brief 显示关于游戏信息
 */
void show_about_game()
{
    clear_screen();
    display_about();
    pause_for_input("\n按任意键返回主菜单...");
}

/**
 * @brief 启动图形化界面
 */
void run_gui_mode()
{
    if (init_gui() == 0)
    {
        printf("启动图形化界面...\n");
        printf("图形化界面已启动，窗口应该可见\n");
        printf("如果看不到窗口，请检查任务栏或按Alt+Tab切换\n");
        while (gui_running && handle_events())
        {
            render_game();
            SDL_Delay(16); // 约60FPS
        }
        printf("退出图形化界面\n");
        cleanup_gui();
    }
    else
    {
        printf("图形化界面启动失败！请检查SDL3库是否正确安装。\n");
        pause_for_input("按任意键返回主菜单...");
    }
}

/**
 * @brief 处理网络玩家回合
 */
bool handle_network_player_turn(int current_player, bool is_local_turn)
{
    if (is_local_turn)
    {
        // 本地玩家回合
        int x, y;
        time_t start_time, end_time;
        
        if (use_timer)
        {
            time(&start_time);
        }

        
        while (1)
        {
            printf("\n轮到你了，请输入落子坐标(行 列，1~%d)，或输入R/r悔棋，S/s认输: ", BOARD_SIZE);
            
            bool input_received = false;
            while (!input_received)
            {
                if (use_timer)
                {
                    time(&end_time);
                    if (difftime(end_time, start_time) > time_limit)
                    {
                        printf("\n你超时了，对方获胜！\n");
                        send_surrender(); // 发送认输消息
                        return 0; // 游戏结束
                    }
                }
                
                int parse_result = parse_network_player_input(&x, &y);
                if (parse_result == 1) // 有效坐标输入
                {
                    input_received = true;
                }
                else if (parse_result == -1) // 认输命令
                {
                    printf("\n你选择认输，对方获胜！\n");
                    send_surrender();
                    return 0; // 游戏结束
                }
                else if (parse_result == 2) // 悔棋成功
                {
                    return 2; // 悔棋发生，需要重新开始回合
                }
                else // parse_result == 0, 特殊命令已处理或无效输入
                {
                    // 继续等待输入
                    continue;
                }
            }
            
            x--; y--; // 转换为0-based坐标
            
            if (player_move(x, y, current_player))
            {
                break; // 成功落子，跳出循环
            }
            else
            {
                printf("坐标无效！请重新输入。\n");
                // 继续循环，重新输入坐标
            }
        }
        
        // 发送落子消息
        if (!send_move(x, y, current_player))
        {
            printf("发送落子消息失败！\n");
            return 0; // 游戏结束
        }
        
        print_board();
        
        if (check_win(x, y, current_player))
        {
            printf("\n你获胜了！\n");
            return 0; // 游戏结束
        }
        
    }
    else
    {
        // 等待对方落子
        printf("\n等待对方落子...\n");
        
        NetworkMessage msg;
        time_t start_time = time(NULL);
        
        while (1)
        {
            if (receive_network_message(&msg, 1000))
            { 
                // 1秒超时
                if (msg.type == MSG_MOVE && msg.player_id == current_player)
                {
                    // 收到落子消息
                    if (!player_move(msg.x, msg.y, current_player))
                    {
                        printf("收到无效的落子坐标！\n");
                        return 0; // 游戏结束
                    }
                    
                    printf("对方落子: (%d, %d)\n", msg.x + 1, msg.y + 1);
                    print_board();
                    
                    if (check_win(msg.x, msg.y, current_player))
                    {
                        printf("\n对方获胜！\n");
                        return 0; // 游戏结束
                    }
                    break;
                    
                } 
                else if (msg.type == MSG_SURRENDER)
                {
                    printf("\n对方认输，你获胜了！\n");
                    return 0; // 游戏结束
                    
                }
                else if (msg.type == MSG_DISCONNECT)
                {
                    printf("\n对方已断开连接\n");
                    return 0; // 游戏结束
                    
                }
                else if (msg.type == MSG_CHAT)
                {
                    printf("[对方]: %s\n", msg.message);
                    
                }
                else if (msg.type == MSG_UNDO_REQUEST)
                {
                    int steps = msg.x;
                    printf("\n对方请求悔棋 %d 步，是否同意？(1:同意/0:拒绝): ", steps);
                    int response = get_integer_input("", 0, 1);
                    
                    if (response && return_move(steps * 2))
                    {
                        printf("同意悔棋，双方各退 %d 步\n", steps);
                        send_undo_response(true, steps);
                        print_board();
                        // 悔棋后需要重新开始当前回合，不改变current_player
                        return 2; // 悔棋发生，需要重新开始回合
                    }
                    else
                    {
                        printf("拒绝悔棋\n");
                        send_undo_response(false, steps);
                        // 继续等待对方落子
                    }
                }
            }
            
            // 检查超时
            if (use_timer && difftime(time(NULL), start_time) > time_limit)
            {
                printf("\n对方超时，你获胜！\n");
                return 0; // 游戏结束
            }
            
            // 检查网络连接
            if (!is_network_connected())
            {
                printf("\n网络连接断开\n");
                return 0; // 游戏结束
            }
        }
    }
    
    return 1; // 正常回合完成
}

/**
 * @brief 网络游戏主循环
 */
bool network_game_loop()
{
    int current_player = PLAYER1; // 总是从玩家1开始
    
    while (1)
    {
        bool is_local_turn = (current_player == network_state.local_player_id);
        
        int turn_result = handle_network_player_turn(current_player, is_local_turn);
        if (turn_result == 0) // 游戏结束
        {
            return true;
        }
        else if (turn_result == 2) // 悔棋发生，重新开始当前回合
        {
            continue; // 不切换玩家，重新开始当前回合
        }
        
        // 检查平局
        if (step_count == BOARD_SIZE * BOARD_SIZE)
        {
            printf("\n平局！\n");
            return true;
        }
        
        // 切换玩家
        current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
        
        // 检查网络连接
        if (!is_network_connected())
        {
            printf("\n网络连接断开\n");
            return false;
        }
    }
    
    return true;
}