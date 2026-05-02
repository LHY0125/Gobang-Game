/**
 * @file gui.h
 * @brief 图形化用户界面头文件
 * @note 使用IUP库实现五子棋的图形化界面
 * @author 刘航宇
 */

#ifndef GUI_H
#define GUI_H

#include "gobang.h"
#include "config.h"
#include "globals.h"

// GUI函数声明

/**
 * @brief 初始化GUI
 * @details 初始化IUP图形库和游戏界面组件
 * @return 成功返回0，失败返回-1
 */
int init_gui();

/**
 * @brief 清理GUI资源
 */
void cleanup_gui();

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

/**
 * @brief 运行图形化界面模式
 * @details 主循环处理事件、渲染画面和更新状态
 */
void run_gui_mode();

#endif // GUI_H
