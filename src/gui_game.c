#include "gui.h"
#include "gui_internal.h"
#include "gui_menu.h"
#include "globals.h"
#include "gobang.h"
#include "ai.h"
#include "record.h"
#include <iup.h>
#include <iupdraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief ACTION 回调：负责重绘
 */
int action_cb(Ihandle *ih)
{
    IupDrawBegin(ih);

    int w, h;
    IupGetIntInt(ih, "DRAWSIZE", &w, &h);

    set_draw_color(ih, 240, 217, 181); // 棋盘背景色 (木纹色近似)
    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    IupDrawRectangle(ih, 0, 0, w, h);

    draw_board_iup(ih);
    draw_stones_iup(ih);

    IupDrawEnd(ih);
    return IUP_DEFAULT;
}

/**
 * @brief 悔棋按钮回调
 */
int btn_undo_cb(Ihandle *ih)
{
    (void)ih;
    if (game_over)
        return IUP_DEFAULT;

    int steps_to_undo = 1;
    if (gui_game_mode == 1) // PvE
    {
        steps_to_undo = 2; // 悔棋两步（玩家+AI）
    }

    if (step_count >= steps_to_undo)
    {
        return_move(steps_to_undo);

        // 更新当前玩家
        if (step_count % 2 == 0)
            current_player_gui = PLAYER;
        else
            current_player_gui = AI; // or PLAYER2

        sprintf(status_message, "已悔棋");
        update_ui_labels();
        IupUpdate(board_canvas);
    }
    else
    {
        sprintf(status_message, "无法悔棋");
        update_ui_labels();
    }
    return IUP_DEFAULT;
}

/**
 * @brief 保存按钮回调
 */
int btn_save_cb(Ihandle *ih)
{
    (void)ih;

    Ihandle *file_dlg = IupFileDlg();
    IupSetAttribute(file_dlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(file_dlg, "TITLE", "保存游戏记录");
    IupSetAttribute(file_dlg, "FILTER", "*.csv");
    IupSetAttribute(file_dlg, "FILTERINFO", "CSV Files");

    IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(file_dlg, "STATUS") != -1)
    {
        char *filename = IupGetAttribute(file_dlg, "VALUE");

        char *base_name = strrchr(filename, '\\');
        if (!base_name)
            base_name = strrchr(filename, '/');
        if (base_name)
            base_name++;
        else
            base_name = filename;

        int mode = (gui_game_mode == 0) ? GAME_MODE_PVP : GAME_MODE_AI;
        if (save_game_to_file(base_name, mode) == 0)
        {
            sprintf(status_message, "保存成功: %s", base_name);
        }
        else
        {
            sprintf(status_message, "保存失败");
        }
        update_ui_labels();
    }

    IupDestroy(file_dlg);
    return IUP_DEFAULT;
}

/**
 * @brief 返回菜单回调
 */
int btn_back_cb(Ihandle *ih)
{
    (void)ih;
    printf("DEBUG: Back to Menu clicked\n");

    // 1. 先显示主菜单
    show_main_menu();
    printf("DEBUG: Main menu shown\n");

    // 2. 销毁游戏窗口
    if (dlg)
    {
        Ihandle *old_dlg = dlg;
        dlg = NULL; // 先清除全局指针
        IupDestroy(old_dlg);
        printf("DEBUG: Destroyed game window\n");
    }

    return IUP_IGNORE; // 返回 IUP_IGNORE 以阻止默认处理
}

/**
 * @brief 鼠标点击回调
 */
int button_cb(Ihandle *ih, int button, int pressed, int x, int y, char *status)
{
    (void)status; // 未使用
    if (gui_game_mode == 2)
        return IUP_DEFAULT; // 复盘模式禁用点击

    if (button == IUP_BUTTON1 && pressed)
    { // 左键按下
        if (game_over)
            return IUP_DEFAULT;

        int board_x, board_y;
        if (screen_to_board(x, y, &board_x, &board_y))
        {
            if (have_space(board_x, board_y))
            {
                // 执行落子操作
                if (player_move(board_x, board_y, current_player_gui))
                {
                    // 检查是否获胜
                    if (check_win(board_x, board_y, current_player_gui))
                    {
                        game_over = 1;
                        if (current_player_gui == PLAYER)
                        {
                            sprintf(status_message, "黑子获胜！");
                            IupMessage("游戏结束", "黑子获胜！");
                        }
                        else
                        {
                            sprintf(status_message, "白子获胜！");
                            IupMessage("游戏结束", "白子获胜！");
                        }
                    }
                    else
                    {
                        if (gui_game_mode == 0) // PvP
                        {
                            current_player_gui = (current_player_gui == PLAYER) ? AI : PLAYER; // AI 在这里表示玩家2
                            if (current_player_gui == PLAYER)
                                sprintf(status_message, "轮到黑子");
                            else
                                sprintf(status_message, "轮到白子");
                        }
                        else // PvE
                        {
                            current_player_gui = AI;
                            sprintf(status_message, "AI思考中...");
                            update_ui_labels();
                            IupUpdate(ih); // 立即更新显示
                            IupFlush();    // 强制刷新事件队列

                            // AI 回合
                            ai_move(ai_difficulty);

                            Step last_step = steps[step_count - 1];
                            if (check_win(last_step.x, last_step.y, AI))
                            {
                                game_over = 1;
                                sprintf(status_message, "AI获胜！");
                                IupMessage("游戏结束", "AI获胜！");
                            }
                            else
                            {
                                current_player_gui = PLAYER;
                                sprintf(status_message, "轮到玩家");
                            }
                        }
                    }
                    update_ui_labels();
                    IupUpdate(ih); // 请求重绘
                }
            }
            else
            {
                sprintf(status_message, "无效位置！");
                update_ui_labels();
            }
        }
    }
    return IUP_DEFAULT;
}

/**
 * @brief 键盘回调
 */
int k_any_cb(Ihandle *ih, int c)
{
    (void)ih;
    if (c == K_ESC)
    {
        if (dlg && IupGetInt(dlg, "VISIBLE"))
        {
            btn_back_cb(ih); // 调用返回菜单逻辑
            return IUP_DEFAULT;
        }
    }
    return IUP_DEFAULT;
}

/**
 * @brief 创建游戏窗口
 */
void create_game_window()
{
    printf("DEBUG: create_game_window start\n");

    if (dlg)
    {
        IupDestroy(dlg);
        dlg = NULL;
    }

    // 创建Canvas (棋盘)
    board_canvas = IupCanvas(NULL);
    if (!board_canvas)
        printf("ERROR: Failed to create board_canvas\n");

    IupSetAttribute(board_canvas, "ACTION", "action_cb");
    IupSetCallback(board_canvas, "ACTION", (Icallback)action_cb);
    IupSetCallback(board_canvas, "BUTTON_CB", (Icallback)button_cb);
    IupSetCallback(board_canvas, "K_ANY", (Icallback)k_any_cb);

    // 计算棋盘像素大小
    int board_pixel_size = BOARD_SIZE * CELL_SIZE + BOARD_OFFSET_X * 2;
    char size[32];
    sprintf(size, "%dx%d", board_pixel_size, board_pixel_size);
    IupSetAttribute(board_canvas, "RASTERSIZE", size);
    IupSetAttribute(board_canvas, "EXPAND", "NO");

    // 创建标签 (玩家信息和游戏状态)
    lbl_player = IupLabel("当前玩家: 黑子");
    lbl_status = IupLabel("准备开始");

    Ihandle *vbox_controls;

    if (gui_game_mode == 2) // 复盘模式
    {
        Ihandle *btn_prev = IupButton("上一步 (Prev)", NULL);
        IupSetCallback(btn_prev, "ACTION", (Icallback)btn_replay_prev_cb);
        IupSetAttribute(btn_prev, "SIZE", "100x30");

        Ihandle *btn_next = IupButton("下一步 (Next)", NULL);
        IupSetCallback(btn_next, "ACTION", (Icallback)btn_replay_next_cb);
        IupSetAttribute(btn_next, "SIZE", "100x30");

        Ihandle *btn_back = IupButton("返回菜单", NULL);
        IupSetCallback(btn_back, "ACTION", (Icallback)btn_back_cb);
        IupSetAttribute(btn_back, "SIZE", "100x30");

        vbox_controls = IupVbox(
            lbl_player,
            lbl_status,
            IupLabel(NULL), // Spacer
            btn_prev,
            btn_next,
            IupLabel(NULL), // Spacer
            btn_back,
            NULL);
    }
    else // 游戏模式 (PvP / PvE)
    {
        Ihandle *btn_undo = IupButton("悔棋 (Undo)", NULL);
        IupSetCallback(btn_undo, "ACTION", (Icallback)btn_undo_cb);
        IupSetAttribute(btn_undo, "SIZE", "100x30");

        Ihandle *btn_save = IupButton("保存 (Save)", NULL);
        IupSetCallback(btn_save, "ACTION", (Icallback)btn_save_cb);
        IupSetAttribute(btn_save, "SIZE", "100x30");

        Ihandle *btn_back = IupButton("返回菜单", NULL);
        IupSetCallback(btn_back, "ACTION", (Icallback)btn_back_cb);
        IupSetAttribute(btn_back, "SIZE", "100x30");

        vbox_controls = IupVbox(
            lbl_player,
            lbl_status,
            IupLabel(NULL), // Spacer
            btn_undo,
            btn_save,
            IupLabel(NULL), // Spacer
            btn_back,
            NULL);
    }

    IupSetAttribute(vbox_controls, "GAP", "15");
    IupSetAttribute(vbox_controls, "MARGIN", "10x10");
    IupSetAttribute(vbox_controls, "ALIGNMENT", "ACENTER");

    Ihandle *hbox_main = IupHbox(board_canvas, vbox_controls, NULL);
    IupSetAttribute(hbox_main, "MARGIN", "10x10");
    IupSetAttribute(hbox_main, "GAP", "10");

    // 创建Dialog
    dlg = IupDialog(hbox_main);
    if (!dlg)
        printf("ERROR: Failed to create dialog\n");

    IupSetAttribute(dlg, "TITLE", "五子棋 - IUP版本");
    IupSetAttribute(dlg, "RESIZE", "NO");

    // 设置 CLOSE_CB 回调，确保点击X也能正确返回菜单
    IupSetCallback(dlg, "CLOSE_CB", (Icallback)btn_back_cb);

    printf("DEBUG: create_game_window end\n");
}

void start_pvp_game_gui()
{
    gui_game_mode = 0;
    empty_board();
    current_player_gui = PLAYER;
    game_over = 0;

    create_game_window();

    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
    sprintf(status_message, "玩家对战模式 - 黑方先行");
    update_ui_labels();
    if (board_canvas)
        IupUpdate(board_canvas);
}

void start_pve_game_gui()
{
    printf("DEBUG: start_pve_game_gui start\n");
    gui_game_mode = 1;
    // ai_difficulty 是全局变量
    empty_board();
    current_player_gui = PLAYER;
    game_over = 0;

    create_game_window();
    printf("DEBUG: create_game_window returned\n");

    if (dlg)
    {
        IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
        printf("DEBUG: IupShowXY called\n");
    }
    else
    {
        printf("ERROR: dlg is NULL in start_pve_game_gui\n");
        return;
    }

    sprintf(status_message, "人机对战模式 - 玩家执黑先行");
    update_ui_labels();
    printf("DEBUG: update_ui_labels returned\n");

    // 强制初始重绘
    if (board_canvas)
    {
        IupUpdate(board_canvas);
    }
    printf("DEBUG: start_pve_game_gui end\n");
}
