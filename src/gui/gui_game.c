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
 * @brief ACTION 回调：负责重绘
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

    // 预创建所有 GDI 对象（避免循环内反复创建销毁）
    HBRUSH bg_brush = CreateSolidBrush(RGB(240, 217, 181));
    HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH white_brush = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH red_brush = CreateSolidBrush(RGB(255, 0, 0));
    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN stone_pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    // 1. 填充背景
    FillRect(hdc, &rc, bg_brush);

    // 2. 绘制棋盘网格
    HPEN prev_pen = (HPEN)SelectObject(hdc, grid_pen);
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        MoveToEx(hdc, BOARD_OFFSET_X, BOARD_OFFSET_Y + i * CELL_SIZE, NULL);
        LineTo(hdc, BOARD_OFFSET_X + (BOARD_SIZE - 1) * CELL_SIZE, BOARD_OFFSET_Y + i * CELL_SIZE);
        MoveToEx(hdc, BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y, NULL);
        LineTo(hdc, BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + (BOARD_SIZE - 1) * CELL_SIZE);
    }

    // 3. 星位/天元
    SelectObject(hdc, black_brush);
    if (BOARD_SIZE == 15)
    {
        int stars[] = {3, 7, 11};
        for (int si = 0; si < 3; si++)
            for (int sj = 0; sj < 3; sj++)
            {
                int cx = BOARD_OFFSET_X + stars[si] * CELL_SIZE;
                int cy = BOARD_OFFSET_Y + stars[sj] * CELL_SIZE;
                Ellipse(hdc, cx - 3, cy - 3, cx + 4, cy + 4);
            }
    }
    else
    {
        int cx = BOARD_OFFSET_X + (BOARD_SIZE / 2) * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + (BOARD_SIZE / 2) * CELL_SIZE;
        Ellipse(hdc, cx - 3, cy - 3, cx + 4, cy + 4);
    }

    // 4. 绘制棋子
    SelectObject(hdc, stone_pen);
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == EMPTY)
                continue;

            int cx = BOARD_OFFSET_X + j * CELL_SIZE;
            int cy = BOARD_OFFSET_Y + i * CELL_SIZE;

            if (board[i][j] == PLAYER)
                SelectObject(hdc, black_brush);
            else
                SelectObject(hdc, white_brush);

            Ellipse(hdc, cx - STONE_RADIUS, cy - STONE_RADIUS,
                    cx + STONE_RADIUS + 1, cy + STONE_RADIUS + 1);
        }

    // 5. 标记最后落子位置（红色小方块）
    if (step_count > 0 && step_count <= MAX_STEPS)
    {
        Step last = steps[step_count - 1];
        int cx = BOARD_OFFSET_X + last.y * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + last.x * CELL_SIZE;
        RECT mark = {cx - 3, cy - 3, cx + 4, cy + 4};
        FillRect(hdc, &mark, red_brush);
    }

    // 恢复原始 GDI 对象，然后清理
    SelectObject(hdc, prev_pen);
    ReleaseDC(hwnd, hdc);

    DeleteObject(bg_brush);
    DeleteObject(black_brush);
    DeleteObject(white_brush);
    DeleteObject(red_brush);
    DeleteObject(grid_pen);
    DeleteObject(stone_pen);

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

    // 计算棋盘像素大小
    int board_pixel_size = BOARD_SIZE * CELL_SIZE + BOARD_OFFSET_X * 2;
    char size[32];
    sprintf(size, "%dx%d", board_pixel_size, board_pixel_size);
    IupSetAttribute(board_canvas, "RASTERSIZE", size);
    IupSetAttribute(board_canvas, "EXPAND", "NO");
    IupSetAttribute(board_canvas, "BORDER", "NO");
    IupSetAttribute(board_canvas, "BGCOLOR", "240 217 181");

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
