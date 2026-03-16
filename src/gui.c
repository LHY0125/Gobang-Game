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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Ihandle *dlg = NULL;
static Ihandle *canvas = NULL;
static int gui_loop_running = 0;

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
                { // 黑子
                    set_draw_color(ih, 0, 0, 0);
                    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
                    IupDrawArc(ih, cx - STONE_RADIUS, cy - STONE_RADIUS, cx + STONE_RADIUS, cy + STONE_RADIUS, 0.0, 360.0);
                }
                else
                { // 白子
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
}

// 绘制UI元素
static void draw_ui_iup(Ihandle *ih)
{
    int infoX = BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE + 20;
    int infoY = BOARD_OFFSET_Y;
    int infoW = 220;
    int infoH = 120;

    // 背景
    set_draw_color(ih, 200, 200, 200); // Light Gray
    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    IupDrawRectangle(ih, infoX, infoY, infoX + infoW, infoY + infoH);

    // 边框
    set_draw_color(ih, 0, 0, 0);
    IupSetAttribute(ih, "DRAWSTYLE", "STROKE");
    IupDrawRectangle(ih, infoX, infoY, infoX + infoW, infoY + infoH);

    // 文本
    // 注意：IUP使用系统字体，不支持直接加载TTF文件像Raylib那样方便
    // 我们可以尝试设置字体属性
    IupSetAttribute(ih, "DRAWFONT", "SimHei, 14");

    IupDrawText(ih, "当前玩家:", -1, infoX + 20, infoY + 20, -1, -1);

    int indicatorX = infoX + 30;
    int indicatorY = infoY + 30;

    if (!game_over)
    {
        if (current_player_gui == PLAYER)
        {
            IupDrawText(ih, "黑子", -1, indicatorX + 20, indicatorY + 10, -1, -1);
            set_draw_color(ih, 0, 0, 0);
            IupSetAttribute(ih, "DRAWSTYLE", "FILL");
            IupDrawArc(ih, indicatorX, indicatorY + 20 - 5, indicatorX + 10, indicatorY + 20 + 5, 0, 360);
        }
        else
        {
            IupDrawText(ih, "白子", -1, indicatorX + 20, indicatorY + 10, -1, -1);
            set_draw_color(ih, 255, 255, 255);
            IupSetAttribute(ih, "DRAWSTYLE", "FILL");
            IupDrawArc(ih, indicatorX, indicatorY + 20 - 5, indicatorX + 10, indicatorY + 20 + 5, 0, 360);
            set_draw_color(ih, 0, 0, 0);
            IupSetAttribute(ih, "DRAWSTYLE", "STROKE");
            IupDrawArc(ih, indicatorX, indicatorY + 20 - 5, indicatorX + 10, indicatorY + 20 + 5, 0, 360);
        }
    }
    else
    {
        set_draw_color(ih, 255, 0, 0);
        IupDrawText(ih, "游戏结束", -1, indicatorX, indicatorY + 10, -1, -1);
    }

    set_draw_color(ih, 0, 0, 0);
    IupDrawText(ih, status_message, -1, infoX + 10, infoY + 60, -1, -1);
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

// ACTION 回调：负责重绘
static int action_cb(Ihandle *ih)
{
    IupDrawBegin(ih);

    int w, h;
    IupGetIntInt(ih, "DRAWSIZE", &w, &h);

    set_draw_color(ih, 245, 245, 245); // Background (Raylib RAYWHITE approx)
    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    IupDrawRectangle(ih, 0, 0, w, h);

    draw_board_iup(ih);
    draw_stones_iup(ih);
    draw_ui_iup(ih);

    IupDrawEnd(ih);
    return IUP_DEFAULT;
}

// 鼠标点击回调
static int button_cb(Ihandle *ih, int button, int pressed, int x, int y, char *status)
{
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
                        }
                        else
                        {
                            sprintf(status_message, "白子获胜！");
                        }
                    }
                    else
                    {
                        // 切换玩家
                        current_player_gui = (current_player_gui == PLAYER) ? AI : PLAYER;
                        if (current_player_gui == PLAYER)
                        {
                            sprintf(status_message, "轮到黑子");
                        }
                        else
                        {
                            sprintf(status_message, "轮到白子");
                        }
                    }
                    IupUpdate(ih); // 请求重绘
                }
            }
            else
            {
                sprintf(status_message, "无效位置！");
                IupUpdate(ih);
            }
        }
    }
    return IUP_DEFAULT;
}

// 键盘回调
static int k_any_cb(Ihandle *ih, int c)
{
    if (c == K_ESC)
    {
        gui_loop_running = 0;
        return IUP_CLOSE;
    }
    return IUP_DEFAULT;
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

    // 创建Canvas
    canvas = IupCanvas(NULL);
    IupSetAttribute(canvas, "ACTION", "action_cb");
    IupSetCallback(canvas, "ACTION", (Icallback)action_cb);
    IupSetCallback(canvas, "BUTTON_CB", (Icallback)button_cb);
    IupSetCallback(canvas, "K_ANY", (Icallback)k_any_cb);

    // 设置Canvas大小
    char size[32];
    sprintf(size, "%dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
    IupSetAttribute(canvas, "RASTERSIZE", size);
    IupSetAttribute(canvas, "EXPAND", "NO");

    // 创建Dialog
    dlg = IupDialog(canvas);
    IupSetAttribute(dlg, "TITLE", "五子棋 - IUP版本");
    IupSetAttribute(dlg, "RESIZE", "NO");

    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

    // 初始化游戏状态
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }
    current_player_gui = PLAYER;
    game_over = 0;
    gui_loop_running = 1;
    sprintf(status_message, "游戏开始");

    printf("图形化界面初始化成功！(IUP)\n");
    printf("使用鼠标点击棋盘进行落子\n");
    printf("按ESC键退出游戏\n");

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

    // 执行一次IUP循环迭代
    int ret = IupLoopStep();
    if (ret == IUP_CLOSE)
    {
        gui_loop_running = 0;
        return 0;
    }

    // 如果窗口被关闭（点X）
    // 注意：IupLoopStep会自动处理窗口关闭并返回IUP_CLOSE，除非定义了CLOSE_CB

    return 1;
}

/**
 * @brief 渲染游戏画面
 */
void render_game()
{
    // 主动刷新Canvas
    // 注意：频繁调用IupUpdate可能会导致闪烁或高CPU占用，但在单线程循环模型中通常是必要的
    // 以确保动画或状态更新能及时反映。
    // 在我们的例子中，主要依靠 button_cb 触发 IupUpdate。
    // 但为了兼容原来的循环结构，我们可以保持这个函数。
    // IupUpdate(canvas);
    // 实际上不需要每一帧都 Update，只有状态变了才需要。
    // 原来的Raylib代码是每帧都重绘。为了性能，这里我们什么都不做，让事件驱动。
    // 除非有外部事件改变了游戏状态（比如网络消息），那时应该调用 IupUpdate。
    // 鉴于目前是本地PVP/AI，所有状态改变都在 button_cb 里，那里已经调用了 IupUpdate。
}

/**
 * @brief 绘制棋盘 (保留空函数以兼容接口，实际绘图在 action_cb 中)
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
    if (canvas)
        IupUpdate(canvas);
    printf("%s\n", message);
}