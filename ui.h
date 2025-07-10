#ifndef UI_H
#define UI_H

#include "gobang.h"

/**
 * @brief UI模块 - 用户界面相关功能
 * @author 刘航宇
 * @date 2025-01-15
 * @version 1.0
 */

/**
 * @brief 显示游戏主菜单
 */
void display_main_menu();

/**
 * @brief 显示棋盘
 */
void display_board();

/**
 * @brief 显示游戏状态信息
 * @param current_player 当前玩家
 * @param step_count 当前步数
 */
void display_game_status(int current_player, int step_count);

/**
 * @brief 显示获胜信息
 * @param winner 获胜者
 */
void display_winner(int winner);

/**
 * @brief 显示游戏设置菜单
 */
void display_settings_menu();

/**
 * @brief 清屏函数
 */
void clear_screen();

/**
 * @brief 暂停等待用户输入
 * @param prompt 提示信息
 */
void pause_for_input(const char* prompt);

/**
 * @brief 显示游戏规则
 */
void display_game_rules();

/**
 * @brief 显示关于信息
 */
void display_about();

#endif // UI_H