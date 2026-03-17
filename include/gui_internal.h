#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include <iup.h>

// 全局变量声明 (在 gui_core.c 中定义)
extern Ihandle *dlg;
extern Ihandle *board_canvas;
extern Ihandle *lbl_player;
extern Ihandle *lbl_status;
extern int gui_loop_running;
extern int gui_game_mode;      // 0: PvP, 1: PvE, 2: Replay
extern int replay_total_steps; // 复盘总步数

// 绘图函数 (在 gui_draw.c 中定义)
void set_draw_color(Ihandle *ih, unsigned char r, unsigned char g, unsigned char b);
void draw_board_iup(Ihandle *ih);
void draw_stones_iup(Ihandle *ih);

// 核心功能 (在 gui_core.c 中定义)
void update_ui_labels();
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y);

// 游戏窗口 (在 gui_game.c 中定义)
void create_game_window();
int action_cb(Ihandle *ih);
int button_cb(Ihandle *ih, int button, int pressed, int x, int y, char *status);
int k_any_cb(Ihandle *ih, int c);
int btn_back_cb(Ihandle *ih);
int btn_undo_cb(Ihandle *ih);
int btn_save_cb(Ihandle *ih);

// 复盘功能 (在 gui_replay.c 中定义)
void select_replay_file_gui();
int btn_replay_prev_cb(Ihandle *ih);
int btn_replay_next_cb(Ihandle *ih);
int btn_replay_sel_ok_cb(Ihandle *ih);
int btn_replay_sel_cancel_cb(Ihandle *ih);

#endif // GUI_INTERNAL_H
