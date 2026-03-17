/**
 * @file gui.h
 * @brief 图形化用户界面头文件
 * @note 使用Raylib库实现五子棋的图形化界面
 * @author 刘航宇
 * @date 2025-01-15
 */

#ifndef GUI_H
#define GUI_H

#include "gobang.h"
#include "config.h"
#include "globals.h"

// GUI函数声明

/**
 * @brief 初始化GUI
 * @details 初始化Raylib图形库和游戏界面组件
 * @return 成功返回0，失败返回-1
 */
int init_gui();

/**
 * @brief 清理GUI资源
 * @details 关闭窗口
 */
void cleanup_gui();

/**
 * @brief 渲染游戏画面
 * @details 完整的游戏画面渲染流程
 */
void render_game();

/**
 * @brief 处理事件
 * @details 处理所有Raylib事件并执行相应操作
 * @return 继续运行返回1，退出返回0
 */
int handle_events();

/**
 * @brief 绘制棋盘
 */
void draw_board();

/**
 * @brief 绘制棋子
 */
void draw_stones();

/**
 * @brief 绘制UI元素
 */
void draw_ui_elements();

/**
 * @brief 屏幕坐标转棋盘坐标
 */
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y);

/**
 * @brief 显示消息
 */
void show_message(const char *message);

/**
 * @brief 启动玩家对战模式
 */
void start_pvp_game_gui();

/**
 * @brief 启动人机对战模式
 */
void start_pve_game_gui();

/**
 * @brief 启动复盘模式
 */
void start_replay_gui();

#endif // GUI_H