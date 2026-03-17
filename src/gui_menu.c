#include <iup.h>
#include <stdlib.h>
#include <stdio.h>
#include "gui_menu.h"
#include "gui.h"
#include "globals.h"
#include "config.h"

static Ihandle *menu_dlg = NULL;

static int btn_pvp_cb(Ihandle *ih)
{
    (void)ih;
    hide_main_menu();
    start_pvp_game_gui();
    return IUP_DEFAULT;
}

static int btn_pve_cb(Ihandle *ih)
{
    (void)ih;
    hide_main_menu();
    start_pve_game_gui();
    return IUP_DEFAULT;
}

static int btn_replay_cb(Ihandle *ih)
{
    (void)ih;
    hide_main_menu();
    start_replay_gui();
    return IUP_DEFAULT;
}

static int btn_save_settings_cb(Ihandle *ih)
{
    Ihandle *dlg = IupGetDialog(ih);
    
    // Get values
    Ihandle *txt_board_size = IupGetDialogChild(dlg, "BOARD_SIZE");
    Ihandle *tgl_forbidden = IupGetDialogChild(dlg, "FORBIDDEN");
    Ihandle *tgl_timer = IupGetDialogChild(dlg, "TIMER");
    Ihandle *txt_time_limit = IupGetDialogChild(dlg, "TIME_LIMIT");
    Ihandle *lst_ai = IupGetDialogChild(dlg, "AI_DIFFICULTY");
    
    // Update globals
    int new_size = IupGetInt(txt_board_size, "VALUE");
    if (new_size < MIN_BOARD_SIZE) new_size = MIN_BOARD_SIZE;
    if (new_size > MAX_BOARD_SIZE) new_size = MAX_BOARD_SIZE;
    BOARD_SIZE = new_size;
    
    use_forbidden_moves = IupGetInt(tgl_forbidden, "VALUE");
    
    use_timer = IupGetInt(tgl_timer, "VALUE");
    if (use_timer) {
        int minutes = IupGetInt(txt_time_limit, "VALUE");
        if (minutes < 1) minutes = 1;
        time_limit = minutes * 60;
    }
    
    int ai_level = IupGetInt(lst_ai, "VALUE");
    if (ai_level < 1) ai_level = 1;
    if (ai_level > 5) ai_level = 5;
    ai_difficulty = ai_level;
    defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT + (ai_difficulty - 1) * 0.1;
    
    // Save config
    save_game_config();
    
    IupHide(dlg);
    return IUP_DEFAULT;
}

static int btn_cancel_settings_cb(Ihandle *ih)
{
    Ihandle *dlg = IupGetDialog(ih);
    IupHide(dlg);
    return IUP_DEFAULT;
}

static int tgl_timer_cb(Ihandle *ih, int state)
{
    Ihandle *dlg = IupGetDialog(ih);
    Ihandle *txt_time_limit = IupGetDialogChild(dlg, "TIME_LIMIT");
    IupSetAttribute(txt_time_limit, "ACTIVE", state ? "YES" : "NO");
    return IUP_DEFAULT;
}

static int btn_settings_cb(Ihandle *ih)
{
    (void)ih;
    
    // 1. Board Size
    Ihandle *lbl_board_size = IupLabel("棋盘大小 (5-25):");
    Ihandle *txt_board_size = IupText(NULL);
    IupSetAttribute(txt_board_size, "NAME", "BOARD_SIZE");
    IupSetAttribute(txt_board_size, "SPIN", "YES");
    IupSetAttribute(txt_board_size, "SPINMIN", "5");
    IupSetAttribute(txt_board_size, "SPINMAX", "25");
    IupSetInt(txt_board_size, "VALUE", BOARD_SIZE);
    IupSetAttribute(txt_board_size, "SIZE", "50x");
    
    // 2. Forbidden Moves
    Ihandle *tgl_forbidden = IupToggle("启用禁手规则", NULL);
    IupSetAttribute(tgl_forbidden, "NAME", "FORBIDDEN");
    IupSetInt(tgl_forbidden, "VALUE", use_forbidden_moves);
    
    // 3. Timer
    Ihandle *tgl_timer = IupToggle("启用计时器", NULL);
    IupSetAttribute(tgl_timer, "NAME", "TIMER");
    IupSetInt(tgl_timer, "VALUE", use_timer);
    IupSetCallback(tgl_timer, "ACTION", (Icallback)tgl_timer_cb);
    
    // 4. Time Limit
    Ihandle *lbl_time_limit = IupLabel("时间限制 (分钟):");
    Ihandle *txt_time_limit = IupText(NULL);
    IupSetAttribute(txt_time_limit, "NAME", "TIME_LIMIT");
    IupSetAttribute(txt_time_limit, "SPIN", "YES");
    IupSetAttribute(txt_time_limit, "SPINMIN", "1");
    IupSetAttribute(txt_time_limit, "SPINMAX", "60");
    IupSetInt(txt_time_limit, "VALUE", time_limit / 60);
    IupSetAttribute(txt_time_limit, "ACTIVE", use_timer ? "YES" : "NO");
    IupSetAttribute(txt_time_limit, "SIZE", "50x");

    // 5. AI Difficulty
    Ihandle *lbl_ai = IupLabel("AI 难度 (1-5):");
    Ihandle *lst_ai = IupList(NULL);
    IupSetAttribute(lst_ai, "NAME", "AI_DIFFICULTY");
    IupSetAttribute(lst_ai, "DROPDOWN", "YES");
    IupSetAttribute(lst_ai, "1", "1 (简单)");
    IupSetAttribute(lst_ai, "2", "2 (普通)");
    IupSetAttribute(lst_ai, "3", "3 (中等)");
    IupSetAttribute(lst_ai, "4", "4 (困难)");
    IupSetAttribute(lst_ai, "5", "5 (专家)");
    IupSetInt(lst_ai, "VALUE", ai_difficulty);
    IupSetAttribute(lst_ai, "SIZE", "80x");

    // Buttons
    Ihandle *btn_save = IupButton("保存", NULL);
    IupSetCallback(btn_save, "ACTION", (Icallback)btn_save_settings_cb);
    IupSetAttribute(btn_save, "SIZE", "60x");
    
    Ihandle *btn_cancel = IupButton("取消", NULL);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_cancel_settings_cb);
    IupSetAttribute(btn_cancel, "SIZE", "60x");
    
    // Layout
    Ihandle *hbox_board = IupHbox(lbl_board_size, txt_board_size, NULL);
    IupSetAttribute(hbox_board, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_board, "GAP", "10");

    Ihandle *hbox_time = IupHbox(lbl_time_limit, txt_time_limit, NULL);
    IupSetAttribute(hbox_time, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_time, "GAP", "10");

    Ihandle *hbox_ai = IupHbox(lbl_ai, lst_ai, NULL);
    IupSetAttribute(hbox_ai, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_ai, "GAP", "10");

    Ihandle *hbox_btns = IupHbox(btn_save, btn_cancel, NULL);
    IupSetAttribute(hbox_btns, "GAP", "20");
    IupSetAttribute(hbox_btns, "MARGIN", "10x0");
    IupSetAttribute(hbox_btns, "ALIGNMENT", "ACENTER");
    
    Ihandle *vbox = IupVbox(
        hbox_board,
        tgl_forbidden,
        tgl_timer,
        hbox_time,
        hbox_ai,
        IupLabel(NULL), // Spacer
        hbox_btns,
        NULL);
        
    IupSetAttribute(vbox, "GAP", "15");
    IupSetAttribute(vbox, "MARGIN", "30x30");
    
    Ihandle *dlg = IupDialog(vbox);
    IupSetAttribute(dlg, "TITLE", "游戏设置");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "MINBOX", "NO");
    IupSetAttribute(dlg, "MAXBOX", "NO");
    
    IupPopup(dlg, IUP_CENTER, IUP_CENTER);
    IupDestroy(dlg);
    
    return IUP_DEFAULT;
}

static int btn_exit_cb(Ihandle *ih)
{
    (void)ih;
    cleanup_gui(); // 清理GUI资源
    exit(0);
    return IUP_DEFAULT;
}

void create_main_menu()
{
    if (menu_dlg) return;

    Ihandle *lbl_title = IupLabel("五子棋 (Gobang)");
    IupSetAttribute(lbl_title, "FONT", "SimHei, 24");
    IupSetAttribute(lbl_title, "ALIGNMENT", "ACENTER");

    Ihandle *btn_pvp = IupButton("玩家对战 (PvP)", NULL);
    IupSetCallback(btn_pvp, "ACTION", (Icallback)btn_pvp_cb);
    IupSetAttribute(btn_pvp, "SIZE", "120x30");
    IupSetAttribute(btn_pvp, "FONT", "SimHei, 12");

    Ihandle *btn_pve = IupButton("人机对战 (PvE)", NULL);
    IupSetCallback(btn_pve, "ACTION", (Icallback)btn_pve_cb);
    IupSetAttribute(btn_pve, "SIZE", "120x30");
    IupSetAttribute(btn_pve, "FONT", "SimHei, 12");

    Ihandle *btn_replay = IupButton("复盘模式", NULL);
    IupSetCallback(btn_replay, "ACTION", (Icallback)btn_replay_cb);
    IupSetAttribute(btn_replay, "SIZE", "120x30");
    IupSetAttribute(btn_replay, "FONT", "SimHei, 12");

    Ihandle *btn_settings = IupButton("设置", NULL);
    IupSetCallback(btn_settings, "ACTION", (Icallback)btn_settings_cb);
    IupSetAttribute(btn_settings, "SIZE", "120x30");
    IupSetAttribute(btn_settings, "FONT", "SimHei, 12");

    Ihandle *btn_exit = IupButton("退出", NULL);
    IupSetCallback(btn_exit, "ACTION", (Icallback)btn_exit_cb);
    IupSetAttribute(btn_exit, "SIZE", "120x30");
    IupSetAttribute(btn_exit, "FONT", "SimHei, 12");

    Ihandle *vbox = IupVbox(
        lbl_title,
        btn_pvp,
        btn_pve,
        btn_replay,
        btn_settings,
        btn_exit,
        NULL);
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(vbox, "GAP", "15");
    IupSetAttribute(vbox, "MARGIN", "40x40");

    menu_dlg = IupDialog(vbox);
    IupSetAttribute(menu_dlg, "TITLE", "五子棋 - 主菜单");
    IupSetAttribute(menu_dlg, "RESIZE", "NO");
    IupSetAttribute(menu_dlg, "MINBOX", "NO");
    IupSetAttribute(menu_dlg, "MAXBOX", "NO");
    
    // 设置对话框关闭回调 (点X关闭程序)
    IupSetCallback(menu_dlg, "CLOSE_CB", (Icallback)btn_exit_cb);
}

void show_main_menu()
{
    if (!menu_dlg)
        create_main_menu();
    IupShowXY(menu_dlg, IUP_CENTER, IUP_CENTER);
}

void hide_main_menu()
{
    if (menu_dlg)
        IupHide(menu_dlg);
}