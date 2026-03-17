#include "gui.h"
#include "gui_internal.h"
#include "gui_menu.h"
#include "globals.h"
#include <iup.h>
#include <stdio.h>
#include <string.h>

// 全局变量定义
Ihandle *dlg = NULL;
Ihandle *board_canvas = NULL;
Ihandle *lbl_player = NULL;
Ihandle *lbl_status = NULL;
int gui_loop_running = 0;
int gui_game_mode = 0;      // 0: PvP, 1: PvE, 2: Replay
int replay_total_steps = 0; // 复盘总步数

/**
 * @brief 初始化GUI
 */
int init_gui()
{
    if (IupOpen(NULL, NULL) == IUP_ERROR)
    {
        printf("IupOpen failed\n");
        return -1;
    }

    // 启用UTF-8模式，确保中文正常显示
    IupSetGlobal("UTF8MODE", "YES");

    create_main_menu();
    show_main_menu();

    gui_loop_running = 1;

    printf("图形化界面初始化成功！(IUP)\n");
    return 0;
}

/**
 * @brief 清理GUI资源
 */
void cleanup_gui()
{
    if (dlg)
    {
        IupDestroy(dlg);
        dlg = NULL;
    }
    IupClose();
    printf("图形化界面已关闭\n");
}

/**
 * @brief 运行图形化界面模式
 * @note 包含初始化、主循环和清理
 */
void run_gui_mode()
{
    if (init_gui() == 0)
    {
        IupMainLoop(); // 使用IUP的主循环
        cleanup_gui();
    }
}

/**
 * @brief 显示消息
 */
void show_message(const char *message)
{
    strncpy(status_message, message, sizeof(status_message) - 1);
    status_message[sizeof(status_message) - 1] = '\0';
    update_ui_labels();
    printf("%s\n", message);
}

/**
 * @brief 更新UI标签状态
 */
void update_ui_labels()
{
    if (lbl_player)
    {
        if (gui_game_mode == 2) // 复盘模式
        {
            char buffer[64];
            sprintf(buffer, "进度: %d / %d", step_count, replay_total_steps);
            IupSetAttribute(lbl_player, "TITLE", buffer);
        }
        else
        {
            if (current_player_gui == PLAYER)
                IupSetAttribute(lbl_player, "TITLE", "当前玩家: 黑子 (玩家)");
            else
                IupSetAttribute(lbl_player, "TITLE", "当前玩家: 白子 (AI/玩家2)");
        }
    }

    if (lbl_status)
    {
        IupSetAttribute(lbl_status, "TITLE", status_message);
    }
}

/**
 * @brief 屏幕坐标转棋盘坐标
 */
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y)
{
    int rel_x = screen_x - BOARD_OFFSET_X;
    int rel_y = screen_y - BOARD_OFFSET_Y;

    *board_x = (rel_y + CELL_SIZE / 2) / CELL_SIZE; // 注意：行号对应y
    *board_y = (rel_x + CELL_SIZE / 2) / CELL_SIZE; // 列号对应x

    return (*board_x >= 0 && *board_x < BOARD_SIZE &&
            *board_y >= 0 && *board_y < BOARD_SIZE);
}
