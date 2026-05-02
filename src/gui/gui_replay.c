#include "gui.h"
#include "gui_internal.h"
#include "gui_menu.h"
#include "globals.h"
#include "gobang.h"
#include "record.h"
#include <iup.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 复盘功能函数

/**
 * @brief 复盘文件选择对话框的确定回调
 */
int btn_replay_sel_ok_cb(Ihandle *ih)
{
    Ihandle *dlg_sel = IupGetDialog(ih);
    Ihandle *list = (Ihandle *)IupGetAttribute(dlg_sel, "MY_LIST");
    char *val = IupGetAttribute(list, "VALUE"); // 返回选中项索引
    if (!val)
        return IUP_DEFAULT;

    int index = atoi(val);
    char *selected_file = IupGetAttributeId(list, "", index);

    if (selected_file)
    {
        printf("DEBUG: Loading file %s\n", selected_file);
        if (load_game_from_file(selected_file))
        {
            printf("DEBUG: File loaded successfully\n");
            replay_total_steps = step_count;
            step_count = 0;
            gui_game_mode = 2; // 复盘模式

            // 关闭选择对话框
            IupHide(dlg_sel);

            // 启动游戏窗口
            create_game_window();
            IupShowXY(dlg, IUP_CENTER, IUP_CENTER); // 显示游戏窗口

            sprintf(status_message, "复盘模式 - %s", selected_file);
            update_ui_labels();
            if (board_canvas)
                IupUpdate(board_canvas);

            return IUP_DEFAULT;
        }
        else
        {
            IupMessage("错误", "无法加载复盘文件");
        }
    }

    return IUP_DEFAULT;
}

/**
 * @brief 复盘文件选择对话框的取消回调
 */
int btn_replay_sel_cancel_cb(Ihandle *ih)
{
    Ihandle *dlg_sel = IupGetDialog(ih);
    IupHide(dlg_sel);
    IupDestroy(dlg_sel);
    show_main_menu(); // 返回主菜单
    return IUP_DEFAULT;
}

/**
 * @brief 上一步按钮回调
 */
int btn_replay_prev_cb(Ihandle *ih)
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

/**
 * @brief 下一步按钮回调
 */
int btn_replay_next_cb(Ihandle *ih)
{
    (void)ih;
    if (step_count < replay_total_steps && step_count >= 0) // 确保步数有效
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

/**
 * @brief 选择复盘文件界面
 */
void select_replay_file_gui()
{
    printf("DEBUG: select_replay_file_gui start\n");
    // 列出 records/ 目录下的文件
    char record_files[100][100];
    int file_count = 0;

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("records\\*", &ffd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // 确保不溢出缓冲区
                if (file_count < 100)
                {
                    strncpy(record_files[file_count], ffd.cFileName, 99);
                    record_files[file_count][99] = '\0';
                    file_count++;
                }
                else
                {
                    break;
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
    }
#endif
    printf("DEBUG: Found %d files\n", file_count);

    if (file_count == 0)
    {
        IupMessage("提示", "未找到复盘记录文件 (records/*)");
        return;
    }

    // 创建列表框
    Ihandle *list = IupList(NULL);
    if (!list)
    {
        printf("ERROR: Failed to create list\n");
        return;
    }
    IupSetAttribute(list, "EXPAND", "YES");
    IupSetAttribute(list, "VISIBLELINES", "10");

    for (int i = 0; i < file_count; i++)
    {
        IupSetAttributeId(list, "", i + 1, record_files[i]);
    }
    IupSetAttribute(list, "VALUE", "1"); // 默认选择第一项

    // 创建确定和取消按钮
    Ihandle *btn_ok = IupButton("确定", NULL);
    IupSetCallback(btn_ok, "ACTION", (Icallback)btn_replay_sel_ok_cb);
    IupSetAttribute(btn_ok, "SIZE", "60x30");

    Ihandle *btn_cancel = IupButton("取消", NULL);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_replay_sel_cancel_cb);
    IupSetAttribute(btn_cancel, "SIZE", "60x30");

    Ihandle *vbox = IupVbox(
        IupLabel("请选择复盘文件:"),
        list,
        IupHbox(btn_ok, btn_cancel, NULL),
        NULL);

    IupSetAttribute(vbox, "GAP", "10");
    IupSetAttribute(vbox, "MARGIN", "10x10");
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");

    Ihandle *dlg_sel = IupDialog(vbox);
    if (!dlg_sel)
    {
        printf("ERROR: Failed to create selection dialog\n");
        return;
    }
    IupSetAttribute(dlg_sel, "TITLE", "选择复盘文件");
    IupSetAttribute(dlg_sel, "MINBOX", "NO");
    IupSetAttribute(dlg_sel, "MAXBOX", "NO");
    IupSetAttribute(dlg_sel, "RESIZE", "NO");

    // 存储列表框句柄到对话框属性
    IupSetAttribute(dlg_sel, "MY_LIST", (char *)list);

    // 显示对话框 (模态)
    IupPopup(dlg_sel, IUP_CENTER, IUP_CENTER);
    // 对话框关闭后销毁
    IupDestroy(dlg_sel);
    printf("DEBUG: select_replay_file_gui end\n");
}

/**
 * @brief 启动复盘GUI流程
 */
void start_replay_gui()
{
    select_replay_file_gui();
    // 不要在这里隐藏主菜单，等待文件选择完成
}
