/**
 * @file gui.c
 * @brief 图形化用户界面实现文件
 * @note 使用IUP库实现五子棋的图形化界面
 * @author 刘航宇
 * @date 2025-01-15
 */

#include "gui.h"
#include <iup.h>
#include <iupdraw.h>
#include "ui.h"
#include "globals.h"
#include "init_board.h"
#include "gobang.h"
#include "gui_menu.h"
#include "ai.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Ihandle *dlg = NULL;
static Ihandle *board_canvas = NULL; // 重命名为 board_canvas
static Ihandle *lbl_player = NULL;
static Ihandle *lbl_status = NULL;
static int gui_loop_running = 0;
static int gui_game_mode = 0;      // 0: PvP, 1: PvE, 2: Replay
static int replay_total_steps = 0; // For Replay Mode

// 回调函数
static int action_cb(Ihandle *ih);
static int button_cb(Ihandle *ih, int button, int pressed, int x, int y, char *status);
static int k_any_cb(Ihandle *ih, int c);
void create_game_window(); // 移除前向声明

// 辅助函数：设置绘图颜色
static void set_draw_color(Ihandle *ih, unsigned char r, unsigned char g, unsigned char b)
{
    char color[32];
    sprintf(color, "%d %d %d", r, g, b);
    IupSetAttribute(ih, "DRAWCOLOR", color);
}

// 绘制棋盘
static void draw_board_iup(Ihandle *ih)
{
    set_draw_color(ih, 0, 0, 0); // Black
    IupSetAttribute(ih, "DRAWSTYLE", "STROKE");

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        // 横线
        IupDrawLine(ih,
                    BOARD_OFFSET_X,
                    BOARD_OFFSET_Y + i * CELL_SIZE,
                    BOARD_OFFSET_X + (BOARD_SIZE - 1) * CELL_SIZE,
                    BOARD_OFFSET_Y + i * CELL_SIZE);
        // 竖线
        IupDrawLine(ih,
                    BOARD_OFFSET_X + i * CELL_SIZE,
                    BOARD_OFFSET_Y,
                    BOARD_OFFSET_X + i * CELL_SIZE,
                    BOARD_OFFSET_Y + (BOARD_SIZE - 1) * CELL_SIZE);
    }

    // 星位/天元
    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    int stars[] = {3, 7, 11}; // 15路棋盘的星位坐标
    if (BOARD_SIZE == 15)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                int cx = BOARD_OFFSET_X + stars[i] * CELL_SIZE;
                int cy = BOARD_OFFSET_Y + stars[j] * CELL_SIZE;
                IupDrawRectangle(ih, cx - 3, cy - 3, cx + 3, cy + 3);
            }
        }
    }
    else
    {
        int center = BOARD_SIZE / 2;
        int cx = BOARD_OFFSET_X + center * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + center * CELL_SIZE;
        IupDrawRectangle(ih, cx - 3, cy - 3, cx + 3, cy + 3);
    }
}

// 绘制棋子
static void draw_stones_iup(Ihandle *ih)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
            {
                int cx = BOARD_OFFSET_X + j * CELL_SIZE; // j是x坐标(列)
                int cy = BOARD_OFFSET_Y + i * CELL_SIZE; // i是y坐标(行)

                if (board[i][j] == PLAYER)
                {
                    // 黑子
                    set_draw_color(ih, 0, 0, 0);
                    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
                    IupDrawArc(ih, cx - STONE_RADIUS, cy - STONE_RADIUS, cx + STONE_RADIUS, cy + STONE_RADIUS, 0.0, 360.0);
                }
                else
                {
                    // 白子
                    set_draw_color(ih, 255, 255, 255);
                    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
                    IupDrawArc(ih, cx - STONE_RADIUS, cy - STONE_RADIUS, cx + STONE_RADIUS, cy + STONE_RADIUS, 0.0, 360.0);

                    set_draw_color(ih, 0, 0, 0);
                    IupSetAttribute(ih, "DRAWSTYLE", "STROKE");
                    IupDrawArc(ih, cx - STONE_RADIUS, cy - STONE_RADIUS, cx + STONE_RADIUS, cy + STONE_RADIUS, 0.0, 360.0);
                }
            }
        }
    }

    // 标记最后落子位置 (红色小点)
    if (step_count > 0)
    {
        // 绘制最后一步的标记
        // 最后一步的坐标是 steps[step_count-1]
        // 所以 step_count-1 是最后一步的索引
        // 所以 steps[step_count-1] 是最后一步

        Step last = steps[step_count - 1];
        int cx = BOARD_OFFSET_X + last.y * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + last.x * CELL_SIZE;

        set_draw_color(ih, 255, 0, 0);
        IupSetAttribute(ih, "DRAWSTYLE", "FILL");
        IupDrawRectangle(ih, cx - 3, cy - 3, cx + 3, cy + 3);
    }
}

// 屏幕坐标转棋盘坐标
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y)
{
    int rel_x = screen_x - BOARD_OFFSET_X;
    int rel_y = screen_y - BOARD_OFFSET_Y;

    *board_x = (rel_y + CELL_SIZE / 2) / CELL_SIZE; // 注意：行号对应y
    *board_y = (rel_x + CELL_SIZE / 2) / CELL_SIZE; // 列号对应x

    return (*board_x >= 0 && *board_x < BOARD_SIZE &&
            *board_y >= 0 && *board_y < BOARD_SIZE);
}

// 更新UI标签状态
static void update_ui_labels()
{
    if (lbl_player)
    {
        if (gui_game_mode == 2) // Replay
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

// ACTION 回调：负责重绘
static int action_cb(Ihandle *ih)
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

// 悔棋按钮回调
static int btn_undo_cb(Ihandle *ih)
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

// 保存按钮回调
static int btn_save_cb(Ihandle *ih)
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

// Replay Prev Callback
static int btn_replay_prev_cb(Ihandle *ih)
{
    (void)ih;
    if (step_count > 0)
    {
        step_count--;
        Step s = steps[step_count];
        board[s.x][s.y] = EMPTY;
        sprintf(status_message, "回退一步");
        update_ui_labels();
        IupUpdate(board_canvas);
    }
    return IUP_DEFAULT;
}

// Replay Next Callback
static int btn_replay_next_cb(Ihandle *ih)
{
    (void)ih;
    if (step_count < replay_total_steps)
    {
        Step s = steps[step_count];
        board[s.x][s.y] = s.player;
        step_count++;
        sprintf(status_message, "前进一步");
        update_ui_labels();
        IupUpdate(board_canvas);
    }
    return IUP_DEFAULT;
}

// 返回菜单回调
static int btn_back_cb(Ihandle *ih)
{
    (void)ih;
    if (dlg)
    {
        IupHide(dlg);
        show_main_menu();
    }
    return IUP_DEFAULT;
}

// 鼠标点击回调
static int button_cb(Ihandle *ih, int button, int pressed, int x, int y, char *status)
{
    (void)status; // Unused
    if (gui_game_mode == 2)
        return IUP_DEFAULT; // Replay mode: disable clicks

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
                            current_player_gui = (current_player_gui == PLAYER) ? AI : PLAYER; // AI here means Player 2
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

// 键盘回调
static int k_any_cb(Ihandle *ih, int c)
{
    (void)ih;
    if (c == K_ESC)
    {
        if (dlg && IupGetInt(dlg, "VISIBLE"))
        {
            IupHide(dlg);
            show_main_menu();
            return IUP_DEFAULT;
        }
    }
    return IUP_DEFAULT;
}

// 创建游戏窗口
void create_game_window()
{
    if (dlg)
    {
        IupDestroy(dlg); // 销毁旧窗口
        dlg = NULL;
    }

    // 创建Canvas (Board)
    board_canvas = IupCanvas(NULL);
    IupSetAttribute(board_canvas, "ACTION", "action_cb");
    IupSetCallback(board_canvas, "ACTION", (Icallback)action_cb);
    IupSetCallback(board_canvas, "BUTTON_CB", (Icallback)button_cb);
    IupSetCallback(board_canvas, "K_ANY", (Icallback)k_any_cb);

    // 
    int board_pixel_size = BOARD_SIZE * CELL_SIZE + BOARD_OFFSET_X * 2;
    char size[32];
    sprintf(size, "%dx%d", board_pixel_size, board_pixel_size);
    IupSetAttribute(board_canvas, "RASTERSIZE", size);
    IupSetAttribute(board_canvas, "EXPAND", "NO");

    // 创建标签 (玩家信息和游戏状态)
    lbl_player = IupLabel("当前玩家: 黑子");
    IupSetAttribute(lbl_player, "FONT", "SimHei, 14");

    lbl_status = IupLabel("准备开始");
    IupSetAttribute(lbl_status, "FONT", "SimHei, 12");

    Ihandle *vbox_controls;

    if (gui_game_mode == 2) // Replay
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
    else // Game Mode
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
    IupSetAttribute(dlg, "TITLE", "五子棋 - IUP版本");
    IupSetAttribute(dlg, "RESIZE", "NO");

    IupMap(dlg);
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
    gui_game_mode = 1;
    // ai_difficulty is global
    empty_board();
    current_player_gui = PLAYER;
    game_over = 0;

    create_game_window();

    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
    sprintf(status_message, "人机对战模式 - 玩家执黑先行");
    update_ui_labels();
    if (board_canvas)
        IupUpdate(board_canvas);
}

void start_replay_gui()
{
    // Open file dialog
    Ihandle *file_dlg = IupFileDlg();
    IupSetAttribute(file_dlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(file_dlg, "TITLE", "选择复盘文件");
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

        if (load_game_from_file(base_name)) // returns game_mode (non-zero) on success
        {
            replay_total_steps = step_count;
            step_count = 0;
            // load_game_from_file already cleared board when reading steps, but steps were read into array
            // wait, load_game_from_file calls empty_board() then reads steps.
            // But it doesn't set board array.
            // So board is empty.

            gui_game_mode = 2; // Replay
            create_game_window();

            IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
            sprintf(status_message, "复盘模式 - %s", base_name);
            update_ui_labels();
            if (board_canvas)
                IupUpdate(board_canvas);
        }
        else
        {
            IupMessage("错误", "无法加载复盘文件");
            show_main_menu();
        }
    }
    else
    {
        show_main_menu();
    }

    IupDestroy(file_dlg);
}

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
 * @brief 处理事件
 */
int handle_events()
{
    if (!gui_loop_running)
        return 0;

    int ret = IupLoopStep();
    if (ret == IUP_CLOSE)
    {
        gui_loop_running = 0;
        return 0;
    }

    return 1;
}

/**
 * @brief 渲染游戏画面
 */
void render_game()
{
    // 事件驱动，不需要手动渲染
}

/**
 * @brief 绘制棋盘 (保留空函数以兼容接口)
 */
void draw_board()
{
    // Implemented in action_cb
}

/**
 * @brief 绘制棋子 (保留空函数以兼容接口)
 */
void draw_stones()
{
    // Implemented in action_cb
}

/**
 * @brief 绘制UI元素 (保留空函数以兼容接口)
 */
void draw_ui_elements()
{
    // Implemented in action_cb
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
