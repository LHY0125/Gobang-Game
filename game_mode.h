/**
 * @file game_mode.h
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋游戏框架头文件
 * @version 3.0
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 *
 * @note 本文件定义了五子棋游戏的三种主要模式：
 * 1. AI对战模式
 * 2. 双人对战模式
 * 3. 复盘模式
 */

#ifndef GAME_MODE_H
#define GAME_MODE_H

#include "gobang.h"

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
 * @param current_player 
 * @return true 
 * @return false 
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

#endif // GAME_MODE_H