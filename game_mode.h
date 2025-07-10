/**
 * @file game_mode.h
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋游戏框架头文件
 * @version 4.0
 * @date 2025-07-02
 *
 * @copyright Copyright (c) 2025
 *
 * @note 本文件定义了五子棋游戏的四种主要模式：
 * 1. AI对战模式
 * 2. 双人对战模式
 * 3. 网络对战模式
 * 4. 复盘模式
 */

#ifndef GAME_MODE_H
#define GAME_MODE_H

#include "gobang.h"
#include "config.h"

/**
 * @brief 从用户获取整数输入
 *
 * @param prompt 提示信息
 * @param min 最小值
 * @param max 最大值
 * @return int 输入的整数
 */
int get_integer_input(const char *prompt, int min, int max);

/**
 * @brief 处理玩家回合
 *
 * @param x 玩家输入的横坐标
 * @param y 玩家输入的纵坐标
 * @return true 输入有效
 * @return false 输入无效
 */
bool parse_player_input(int *x, int *y);

/**
 * @brief 处理AI回合
 *
 * @param current_player 当前玩家
 */
bool handle_player_turn(int current_player);

/**
 * @brief AI对战模式
 * 实现玩家与AI的对战逻辑
 */
void run_ai_game();

/**
 * @brief 双人对战模式
 * 实现两个玩家之间的对战逻辑
 */
void run_pvp_game();

/**
 * @brief 复盘模式
 * 加载并重现历史对局
 */
void run_review_mode();

/**
 * @brief 网络对战模式
 * 实现两台设备之间的在线对战
 */
void run_network_game();

/**
 * @brief 处理网络玩家回合
 * @param current_player 当前玩家
 * @param is_local_turn 是否为本地玩家回合
 * @return true 回合处理成功
 * @return false 游戏结束或网络错误
 */
bool handle_network_player_turn(int current_player, bool is_local_turn);

/**
 * @brief 网络游戏主循环
 * @return true 游戏正常结束
 * @return false 网络错误或异常退出
 */
bool network_game_loop();

#endif // GAME_MODE_H