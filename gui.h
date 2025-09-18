/**
 * @file gui.h
 * @brief 图形化用户界面头文件
 * @note 使用SDL3库实现五子棋的图形化界面
 * @author 刘航宇
 * @date 2025-01-15
 */

#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include "gobang.h"

// 窗口和棋盘配置
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BOARD_OFFSET_X 50
#define BOARD_OFFSET_Y 50
#define CELL_SIZE 30
#define STONE_RADIUS 12

// 颜色定义
#define GUI_COLOR_BACKGROUND {240, 217, 181, 255}
#define GUI_COLOR_BOARD_LINE {0, 0, 0, 255}
#define GUI_COLOR_BLACK_STONE {0, 0, 0, 255}
#define GUI_COLOR_WHITE_STONE {255, 255, 255, 255}
#define GUI_COLOR_STONE_BORDER {100, 100, 100, 255}

// GUI函数声明
int init_gui();
void cleanup_gui();
void render_game();
int handle_events();
void draw_board();
void draw_stones();
void draw_ui_elements();
void draw_circle(int center_x, int center_y, int radius, SDL_Color color);
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y);
void show_message(const char *message);

// 全局GUI变量
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern int gui_running;
extern int current_player_gui;
extern int game_over;
extern char status_message[256];

#endif // GUI_H