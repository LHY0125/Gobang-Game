#include "gui_internal.h"
#include "globals.h"
#include "gobang.h" // for BOARD_SIZE, etc.
#include <iup.h>
#include <iupdraw.h>
#include <stdio.h>

/**
 * @brief 设置绘图颜色
 */
void set_draw_color(Ihandle *ih, unsigned char r, unsigned char g, unsigned char b)
{
    char color[32];
    sprintf(color, "%d %d %d", r, g, b);
    IupSetAttribute(ih, "DRAWCOLOR", color);
}

/**
 * @brief 绘制棋盘
 */
void draw_board_iup(Ihandle *ih)
{
    set_draw_color(ih, 0, 0, 0); // 黑色
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

/**
 * @brief 绘制棋子
 */
void draw_stones_iup(Ihandle *ih)
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
    if (step_count > 0 && step_count <= MAX_STEPS)
    {
        // 绘制最后一步的标记
        // 最后一步的坐标是 steps[step_count-1]
        // 所以 step_count-1 是最后一步的索引

        Step last = steps[step_count - 1];
        int cx = BOARD_OFFSET_X + last.y * CELL_SIZE;
        int cy = BOARD_OFFSET_Y + last.x * CELL_SIZE;

        set_draw_color(ih, 255, 0, 0);
        IupSetAttribute(ih, "DRAWSTYLE", "FILL");
        IupDrawRectangle(ih, cx - 3, cy - 3, cx + 3, cy + 3);
    }
}
