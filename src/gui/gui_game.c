#include "gui.h"
#include "gui_internal.h"
#include "gui_menu.h"
#include "globals.h"
#include "gobang.h"
#include "ai.h"
#include "record.h"
#include "network.h"
#include "llm_ai.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <iup.h>
#include <iupdraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Ihandle *timer = NULL; // 网络轮询定时器
static Ihandle *llm_timer = NULL; // LLM异步轮询定时器

/**
 * @brief 网络事件轮询回调
 */
static int timer_cb(Ihandle *ih)
{
    (void)ih;
    if (gui_game_mode != 3 || game_over)
        return IUP_DEFAULT;

    NetworkMessage msg;
    // 非阻塞接收消息
    if (receive_network_message(&msg, 0))
    {
        if (msg.type == MSG_MOVE)
        {
            int bx = msg.x;
            int by = msg.y;
            int pid = msg.player_id;

            if (have_space(bx, by))
            {
                player_move(bx, by, pid);
                if (check_win(bx, by, pid))
                {
                    game_over = 1;
                    sprintf(status_message, "对手获胜！");
                    IupMessage("游戏结束", "对手获胜！");
                }
                else
                {
                    current_player_gui = network_state.local_player_id;
                    sprintf(status_message, "轮到你落子");
                }
                update_ui_labels();
                if (board_canvas)
                    IupUpdate(board_canvas);
            }
        }
        else if (msg.type == MSG_DISCONNECT || msg.type == MSG_SURRENDER)
        {
            game_over = 1;
            sprintf(status_message, "对手已断开连接/认输");
            IupMessage("游戏结束", "对手已退出游戏，你赢了！");
            update_ui_labels();
        }
    }

    if (!is_network_connected() && !game_over)
    {
        game_over = 1;
        sprintf(status_message, "与服务器断开连接");
        IupMessage("错误", "网络连接已断开");
        update_ui_labels();
    }

    return IUP_DEFAULT;
}

/**
 * @brief 处理AI落子结果（LLM或算法）
 */
static void process_ai_move_result(void)
{
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
    update_ui_labels();
    if (board_canvas)
        IupUpdate(board_canvas);
}

/**
 * @brief LLM异步轮询定时器回调
 */
static int llm_timer_cb(Ihandle *ih)
{
    (void)ih;
    int x, y;
    int result = llm_ai_poll_result(&x, &y);

    if (result == 0)
        return IUP_DEFAULT; // 仍在思考

    // 停止轮询定时器
    if (llm_timer)
    {
        IupSetAttribute(llm_timer, "RUN", "NO");
    }

    if (result == 1 && x >= 0 && y >= 0 && player_move(x, y, AI))
    {
        // LLM成功且落子合法
    }
    else
    {
        // LLM失败或坐标非法，回退到算法AI
        if (result == 1)
            snprintf(status_message, sizeof(status_message), "大模型返回非法位置，使用算法AI");
        else
            snprintf(status_message, sizeof(status_message), "大模型响应失败，使用算法AI");
        update_ui_labels();
        ai_move(ai_difficulty);
    }

    process_ai_move_result();
    return IUP_DEFAULT;
}

/**
 * @brief MAP_CB 回调：Canvas映射后强制重绘
 */
static int map_cb(Ihandle *ih)
{
    (void)ih;
    IupUpdate(board_canvas);
    return IUP_DEFAULT;
}

/**
 * @brief ACTION 回调：负责重绘（经典木纹风格）
 */
int action_cb(Ihandle *ih)
{
    HWND hwnd = (HWND)IupGetAttribute(ih, "WID");
    if (!hwnd)
        return IUP_DEFAULT;

    HDC hdc = GetDC(hwnd);
    if (!hdc)
        return IUP_DEFAULT;

    RECT rc;
    GetClientRect(hwnd, &rc);

    // === 预创建所有 GDI 对象 ===
    HBRUSH bg_brush = CreateSolidBrush(RGB(CLR_BOARD_BG_R, CLR_BOARD_BG_G, CLR_BOARD_BG_B));
    HBRUSH black_outer = CreateSolidBrush(RGB(CLR_BLACK_STONE_R, CLR_BLACK_STONE_G, CLR_BLACK_STONE_B));
    HBRUSH black_core = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH black_hl = CreateSolidBrush(RGB(CLR_BLACK_HIGHLIGHT_R, CLR_BLACK_HIGHLIGHT_G, CLR_BLACK_HIGHLIGHT_B));
    HBRUSH white_outer = CreateSolidBrush(RGB(CLR_WHITE_BORDER_R, CLR_WHITE_BORDER_G, CLR_WHITE_BORDER_B));
    HBRUSH white_core = CreateSolidBrush(RGB(CLR_WHITE_STONE_R, CLR_WHITE_STONE_G, CLR_WHITE_STONE_B));
    HBRUSH white_hl = CreateSolidBrush(RGB(CLR_WHITE_HIGHLIGHT_R, CLR_WHITE_HIGHLIGHT_G, CLR_WHITE_HIGHLIGHT_B));
    HBRUSH star_brush = CreateSolidBrush(RGB(CLR_STAR_POINT_R, CLR_STAR_POINT_G, CLR_STAR_POINT_B));
    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(CLR_GRID_LINE_R, CLR_GRID_LINE_G, CLR_GRID_LINE_B));
    HPEN border_pen = CreatePen(PS_SOLID, 2, RGB(CLR_BOARD_BORDER_R, CLR_BOARD_BORDER_G, CLR_BOARD_BORDER_B));
    HPEN last_move_pen = CreatePen(PS_SOLID, 2, RGB(CLR_LAST_MOVE_R, CLR_LAST_MOVE_G, CLR_LAST_MOVE_B));
    HPEN null_pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HFONT hfont_coord = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, 0, 0, 0, 0, "SimHei");

    // 1. 填充背景
    FillRect(hdc, &rc, bg_brush);

    // 2. 绘制棋盘边框（深色外框）
    HPEN prev_pen = (HPEN)SelectObject(hdc, border_pen);
    HBRUSH prev_brush = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    int grid_left = BOARD_OFFSET_X;
    int grid_top = BOARD_OFFSET_Y;
    int grid_right = BOARD_OFFSET_X + (BOARD_SIZE - 1) * CELL_SIZE;
    int grid_bottom = BOARD_OFFSET_Y + (BOARD_SIZE - 1) * CELL_SIZE;
    Rectangle(hdc, grid_left - 2, grid_top - 2, grid_right + 3, grid_bottom + 3);

    // 3. 绘制棋盘网格
    SelectObject(hdc, grid_pen);
    SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        MoveToEx(hdc, grid_left, BOARD_OFFSET_Y + i * CELL_SIZE, NULL);
        LineTo(hdc, grid_right, BOARD_OFFSET_Y + i * CELL_SIZE);
        MoveToEx(hdc, BOARD_OFFSET_X + i * CELL_SIZE, grid_top, NULL);
        LineTo(hdc, BOARD_OFFSET_X + i * CELL_SIZE, grid_bottom);
    }

    // 4. 星位/天元（实心圆）
    SelectObject(hdc, null_pen);
    SelectObject(hdc, star_brush);
    if (BOARD_SIZE == 15)
    {
        int stars[] = {3, 7, 11};
        for (int si = 0; si < 3; si++)
            for (int sj = 0; sj < 3; sj++)
            {
                int cx = BOARD_OFFSET_X + stars[si] * CELL_SIZE;
                int cy = BOARD_OFFSET_Y + stars[sj] * CELL_SIZE;
                Ellipse(hdc, cx - 4, cy - 4, cx + 5, cy + 5);
            }
    }
    else if (BOARD_SIZE >= 9)
    {
        int cx = BOARD_OFFSET_X + (BOARD_SIZE / 2) * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + (BOARD_SIZE / 2) * CELL_SIZE;
        Ellipse(hdc, cx - 4, cy - 4, cx + 5, cy + 5);
    }

    // 5. 绘制坐标标注
    {
        HFONT prev_font = (HFONT)SelectObject(hdc, hfont_coord);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(CLR_BOARD_BORDER_R, CLR_BOARD_BORDER_G, CLR_BOARD_BORDER_B));

        // 列坐标 (A, B, C, ...)
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            char label[2] = {'A' + j, '\0'};
            int tx = BOARD_OFFSET_X + j * CELL_SIZE - 4;
            TextOut(hdc, tx, grid_top - 18, label, 1);
            TextOut(hdc, tx, grid_bottom + 5, label, 1);
        }
        // 行坐标 (1, 2, 3, ...)
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            char label[4];
            int len = snprintf(label, sizeof(label), "%d", i + 1);
            int ty = BOARD_OFFSET_Y + i * CELL_SIZE - 7;
            TextOut(hdc, grid_left - 18 - (len > 1 ? 4 : 0), ty, label, len);
            TextOut(hdc, grid_right + 6, ty, label, len);
        }
        SelectObject(hdc, prev_font);
    }

    // 6. 绘制棋子（渐变效果：3层同心圆）
    SelectObject(hdc, null_pen);
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == EMPTY)
                continue;

            int cx = BOARD_OFFSET_X + j * CELL_SIZE;
            int cy = BOARD_OFFSET_Y + i * CELL_SIZE;

            if (board[i][j] == PLAYER)
            {
                // 黑子：外圈深灰 → 中圈黑 → 中心高光
                SelectObject(hdc, black_outer);
                Ellipse(hdc, cx - STONE_RADIUS, cy - STONE_RADIUS,
                        cx + STONE_RADIUS + 1, cy + STONE_RADIUS + 1);
                SelectObject(hdc, black_core);
                Ellipse(hdc, cx - STONE_RADIUS + 2, cy - STONE_RADIUS + 2,
                        cx + STONE_RADIUS - 1, cy + STONE_RADIUS - 1);
                SelectObject(hdc, black_hl);
                Ellipse(hdc, cx - 4, cy - 5, cx - 1, cy - 2);
            }
            else
            {
                // 白子：外圈灰边 → 中圈白 → 中心高光
                SelectObject(hdc, white_outer);
                Ellipse(hdc, cx - STONE_RADIUS, cy - STONE_RADIUS,
                        cx + STONE_RADIUS + 1, cy + STONE_RADIUS + 1);
                SelectObject(hdc, white_core);
                Ellipse(hdc, cx - STONE_RADIUS + 2, cy - STONE_RADIUS + 2,
                        cx + STONE_RADIUS - 1, cy + STONE_RADIUS - 1);
                SelectObject(hdc, white_hl);
                Ellipse(hdc, cx - 4, cy - 5, cx - 1, cy - 2);
            }
        }

    // 7. 标记最后落子位置（蓝色圆环）
    if (step_count > 0 && step_count <= MAX_STEPS)
    {
        Step last = steps[step_count - 1];
        int cx = BOARD_OFFSET_X + last.y * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + last.x * CELL_SIZE;
        SelectObject(hdc, last_move_pen);
        SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cx - 5, cy - 5, cx + 6, cy + 6);
    }

    // 恢复原始 GDI 对象，然后清理
    SelectObject(hdc, prev_pen);
    SelectObject(hdc, prev_brush);
    ReleaseDC(hwnd, hdc);

    DeleteObject(bg_brush);
    DeleteObject(black_outer);
    DeleteObject(black_core);
    DeleteObject(black_hl);
    DeleteObject(white_outer);
    DeleteObject(white_core);
    DeleteObject(white_hl);
    DeleteObject(star_brush);
    DeleteObject(grid_pen);
    DeleteObject(border_pen);
    DeleteObject(last_move_pen);
    DeleteObject(null_pen);
    DeleteObject(hfont_coord);

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
    else if (gui_game_mode == 3) // Network
    {
        IupMessage("提示", "网络模式暂不支持悔棋");
        return IUP_DEFAULT;
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

        int mode;
        if (gui_game_mode == 0)
            mode = GAME_MODE_PVP;
        else if (gui_game_mode == 3)
            mode = GAME_MODE_NETWORK;
        else
            mode = GAME_MODE_AI;
        if (save_game_to_file(base_name, mode) == 0)
        {
            snprintf(status_message, sizeof(status_message), "保存成功: %s", base_name);
        }
        else
        {
            snprintf(status_message, sizeof(status_message), "保存失败");
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

    // 停止所有定时器
    if (timer)
    {
        IupSetAttribute(timer, "RUN", "NO");
        IupDestroy(timer);
        timer = NULL;
    }
    if (llm_timer)
    {
        IupSetAttribute(llm_timer, "RUN", "NO");
        IupDestroy(llm_timer);
        llm_timer = NULL;
    }

    // 如果是网络模式，彻底清理网络资源
    if (gui_game_mode == 3)
    {
        cleanup_network();
    }

    // 1. 先显示主菜单
    show_main_menu();

    // 2. 销毁游戏窗口
    if (dlg)
    {
        Ihandle *old_dlg = dlg;
        dlg = NULL; // 先清除全局指针
        IupDestroy(old_dlg);
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

        if (gui_game_mode == 3 && current_player_gui != network_state.local_player_id)
        {
            return IUP_DEFAULT; // 网络模式下，非自己回合不可落子
        }

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
                        if (gui_game_mode == 3)
                            send_move(board_x, board_y, current_player_gui); // 发送最后一步
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
                        else if (gui_game_mode == 3) // Network
                        {
                            send_move(board_x, board_y, current_player_gui);
                            current_player_gui = network_state.remote_player_id;
                            sprintf(status_message, "等待对手落子...");
                        }
                        else // PvE
                        {
                            current_player_gui = AI;
                            update_ui_labels();
                            IupUpdate(ih); // 立即更新显示
                            IupFlush();    // 强制刷新事件队列

                            if (llm_use)
                            {
                                // 大模型AI - 异步调用，不阻塞UI
                                sprintf(status_message, "AI思考中（大模型）...");
                                update_ui_labels();

                                // 创建或复用轮询定时器
                                if (!llm_timer)
                                {
                                    llm_timer = IupTimer();
                                    IupSetCallback(llm_timer, "ACTION_CB", (Icallback)llm_timer_cb);
                                    IupSetAttribute(llm_timer, "TIME", "100"); // 100ms轮询
                                }
                                llm_ai_start_move();
                                IupSetAttribute(llm_timer, "RUN", "YES");
                            }
                            else
                            {
                                // 算法AI - 同步调用
                                sprintf(status_message, "AI思考中...");
                                update_ui_labels();

                                ai_move(ai_difficulty);
                                process_ai_move_result();
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
    if (dlg)
    {
        IupDestroy(dlg);
        dlg = NULL;
    }

    // 创建Canvas (棋盘)
    board_canvas = IupCanvas(NULL);
    if (!board_canvas)
        printf("ERROR: Failed to create board_canvas\n");

    IupSetCallback(board_canvas, "ACTION", (Icallback)action_cb);
    IupSetCallback(board_canvas, "BUTTON_CB", (Icallback)button_cb);
    IupSetCallback(board_canvas, "K_ANY", (Icallback)k_any_cb);
    IupSetCallback(board_canvas, "MAP_CB", (Icallback)map_cb);

    // 计算棋盘像素大小（含坐标标注区域）
    int board_pixel_size = (BOARD_SIZE - 1) * CELL_SIZE + BOARD_OFFSET_X * 2 + 20;
    char size[32];
    sprintf(size, "%dx%d", board_pixel_size, board_pixel_size);
    IupSetAttribute(board_canvas, "RASTERSIZE", size);
    IupSetAttribute(board_canvas, "EXPAND", "NO");
    IupSetAttribute(board_canvas, "BORDER", "NO");
    IupSetAttribute(board_canvas, "BGCOLOR", "212 165 116");

    // === 创建标签 ===
    lbl_player = IupLabel("当前玩家: 黑子");
    IupSetAttribute(lbl_player, "FONT", "SimHei, 13");
    IupSetAttribute(lbl_player, "FGCOLOR", CLR_TEXT_TITLE);

    lbl_status = IupLabel("准备开始");
    IupSetAttribute(lbl_status, "FONT", "SimHei, 11");
    IupSetAttribute(lbl_status, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 对局信息面板 ===
    Ihandle *info_vbox = IupVbox(lbl_player, lbl_status, NULL);
    IupSetAttribute(info_vbox, "GAP", "4");
    IupSetAttribute(info_vbox, "MARGIN", "10x8");
    Ihandle *frm_info = IupFrame(info_vbox);
    IupSetAttribute(frm_info, "TITLE", "对局信息");
    IupSetAttribute(frm_info, "FONT", "SimHei, 11");
    IupSetAttribute(frm_info, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 按钮样式宏 ===
#define SET_BTN_STYLE(btn, w, h, font)           \
    IupSetAttribute(btn, "SIZE", #w "x" #h);    \
    IupSetAttribute(btn, "FONT", font);          \
    IupSetAttribute(btn, "BGCOLOR", CLR_BTN_NORMAL_BG); \
    IupSetAttribute(btn, "FGCOLOR", CLR_BTN_NORMAL_FG); \
    IupSetAttribute(btn, "FLAT", "YES")

    Ihandle *vbox_controls;

    if (gui_game_mode == 2) // 复盘模式
    {
        Ihandle *btn_prev = IupButton("上一步", NULL);
        IupSetCallback(btn_prev, "ACTION", (Icallback)btn_replay_prev_cb);
        SET_BTN_STYLE(btn_prev, 120, 35, "SimHei, 11");

        Ihandle *btn_next = IupButton("下一步", NULL);
        IupSetCallback(btn_next, "ACTION", (Icallback)btn_replay_next_cb);
        SET_BTN_STYLE(btn_next, 120, 35, "SimHei, 11");

        Ihandle *hbox_nav = IupHbox(btn_prev, btn_next, NULL);
        IupSetAttribute(hbox_nav, "GAP", "8");
        IupSetAttribute(hbox_nav, "ALIGNMENT", "ACENTER");

        Ihandle *btn_back = IupButton("返回菜单", NULL);
        IupSetCallback(btn_back, "ACTION", (Icallback)btn_back_cb);
        SET_BTN_STYLE(btn_back, 120, 35, "SimHei, 11");

        vbox_controls = IupVbox(
            frm_info,
            hbox_nav,
            btn_back,
            NULL);
    }
    else // 游戏模式 (PvP / PvE / Network)
    {
        Ihandle *btn_undo = IupButton("悔棋", NULL);
        IupSetCallback(btn_undo, "ACTION", (Icallback)btn_undo_cb);
        SET_BTN_STYLE(btn_undo, 120, 35, "SimHei, 11");

        Ihandle *btn_save = IupButton("保存棋谱", NULL);
        IupSetCallback(btn_save, "ACTION", (Icallback)btn_save_cb);
        SET_BTN_STYLE(btn_save, 120, 35, "SimHei, 11");

        Ihandle *btn_back = IupButton("返回菜单", NULL);
        IupSetCallback(btn_back, "ACTION", (Icallback)btn_back_cb);
        SET_BTN_STYLE(btn_back, 120, 35, "SimHei, 11");

        vbox_controls = IupVbox(
            frm_info,
            btn_undo,
            btn_save,
            btn_back,
            NULL);
    }

#undef SET_BTN_STYLE

    IupSetAttribute(vbox_controls, "GAP", "10");
    IupSetAttribute(vbox_controls, "MARGIN", "10x10");
    IupSetAttribute(vbox_controls, "ALIGNMENT", "ACENTER");

    Ihandle *hbox_main = IupHbox(board_canvas, vbox_controls, NULL);
    IupSetAttribute(hbox_main, "MARGIN", "10x10");
    IupSetAttribute(hbox_main, "GAP", "8");

    // 创建Dialog
    dlg = IupDialog(hbox_main);
    if (!dlg)
        printf("ERROR: Failed to create dialog\n");

    IupSetAttribute(dlg, "TITLE", "五子棋");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "BGCOLOR", CLR_WINDOW_BG);

    // 设置 CLOSE_CB 回调，确保点击X也能正确返回菜单
    IupSetCallback(dlg, "CLOSE_CB", (Icallback)btn_back_cb);
}

void start_pvp_game_gui()
{
    gui_game_mode = 0;
    empty_board();
    current_player_gui = PLAYER;
    game_over = 0;

    create_game_window();

    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
    IupFlush(); // 确保窗口完全映射
    sprintf(status_message, "玩家对战模式 - 黑方先行");
    update_ui_labels();
    if (board_canvas)
        IupUpdate(board_canvas);
}

void start_pve_game_gui()
{
    gui_game_mode = 1;
    empty_board();
    current_player_gui = PLAYER;
    game_over = 0;

    create_game_window();

    if (dlg)
    {
        IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
        IupFlush();
    }
    else
    {
        return;
    }

    sprintf(status_message, "人机对战模式 - 玩家执黑先行");
    update_ui_labels();

    if (board_canvas)
        IupUpdate(board_canvas);
}

void start_network_game_gui()
{
    gui_game_mode = 3;
    empty_board();

    current_player_gui = PLAYER1;
    game_over = 0;

    create_game_window();

    if (dlg)
    {
        IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
        IupFlush();
    }

    if (network_state.is_server)
        sprintf(status_message, "局域网联机 - 你是主机(黑子)，轮到你落子");
    else
        sprintf(status_message, "局域网联机 - 你是客机(白子)，等待对手落子...");

    update_ui_labels();

    // 强制初始重绘
    if (board_canvas)
    {
        IupUpdate(board_canvas);
    }

    // 启动网络轮询定时器
    timer = IupTimer();
    IupSetCallback(timer, "ACTION_CB", (Icallback)timer_cb);
    IupSetAttribute(timer, "TIME", "50"); // 50ms 轮询一次
    IupSetAttribute(timer, "RUN", "YES");
}
