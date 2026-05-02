#include <iup.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gui_menu.h"
#include "gui.h"
#include "gui_internal.h"
#include "globals.h"
#include "config.h"
#include "network.h"

Ihandle *menu_dlg = NULL;

static int btn_pvp_cb(Ihandle *ih)
{
    (void)ih;
    start_pvp_game_gui();
    IupHide(menu_dlg);
    return IUP_DEFAULT;
}

static int btn_pve_cb(Ihandle *ih)
{
    (void)ih;
    start_pve_game_gui();
    IupHide(menu_dlg);
    return IUP_DEFAULT;
}

// --- 网络对战相关回调 ---
static int btn_network_host_cb(Ihandle *ih)
{
    Ihandle *dlg = IupGetDialog(ih);
    Ihandle *txt_port = IupGetDialogChild(dlg, "NET_PORT");
    int port = IupGetInt(txt_port, "VALUE");
    if (port <= 0 || port > 65535) port = DEFAULT_NETWORK_PORT;

    if (create_server(port))
    {
        IupMessage("成功", "房间创建成功，等待玩家加入...");
        IupHide(dlg);
        start_network_game_gui();
        IupHide(menu_dlg);
    }
    else
    {
        IupMessage("错误", "创建房间失败，可能是端口被占用");
    }
    return IUP_DEFAULT;
}

static int btn_network_join_cb(Ihandle *ih)
{
    Ihandle *dlg = IupGetDialog(ih);
    Ihandle *txt_ip = IupGetDialogChild(dlg, "NET_IP");
    Ihandle *txt_port = IupGetDialogChild(dlg, "NET_PORT");
    
    char *ip = IupGetAttribute(txt_ip, "VALUE");
    int port = IupGetInt(txt_port, "VALUE");
    if (port <= 0 || port > 65535) port = DEFAULT_NETWORK_PORT;

    if (connect_to_server(ip, port))
    {
        IupMessage("成功", "成功加入房间！");
        IupHide(dlg);
        start_network_game_gui();
        IupHide(menu_dlg);
    }
    else
    {
        IupMessage("错误", "加入房间失败，请检查IP和端口");
    }
    return IUP_DEFAULT;
}

static int btn_network_cancel_cb(Ihandle *ih)
{
    Ihandle *dlg = IupGetDialog(ih);
    IupHide(dlg);
    return IUP_DEFAULT;
}

static int btn_network_cb(Ihandle *ih)
{
    (void)ih;

    Ihandle *lbl_ip = IupLabel("目标 IP:");
    Ihandle *txt_ip = IupText(NULL);
    IupSetAttribute(txt_ip, "NAME", "NET_IP");
    IupSetAttribute(txt_ip, "VALUE", "127.0.0.1");
    IupSetAttribute(txt_ip, "SIZE", "120x");

    Ihandle *hbox_ip = IupHbox(lbl_ip, txt_ip, NULL);
    IupSetAttribute(hbox_ip, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_ip, "GAP", "10");

    Ihandle *lbl_port = IupLabel("端口:");
    Ihandle *txt_port = IupText(NULL);
    IupSetAttribute(txt_port, "NAME", "NET_PORT");
    char port_str[16];
    sprintf(port_str, "%d", DEFAULT_NETWORK_PORT);
    IupSetAttribute(txt_port, "VALUE", port_str);
    IupSetAttribute(txt_port, "SIZE", "60x");

    Ihandle *hbox_port = IupHbox(lbl_port, txt_port, NULL);
    IupSetAttribute(hbox_port, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_port, "GAP", "10");

    Ihandle *vbox_input = IupVbox(hbox_ip, hbox_port, NULL);
    IupSetAttribute(vbox_input, "GAP", "8");
    IupSetAttribute(vbox_input, "MARGIN", "10x8");

    Ihandle *frm_input = IupFrame(vbox_input);
    IupSetAttribute(frm_input, "TITLE", "连接设置");
    IupSetAttribute(frm_input, "FONT", "SimHei, 11");
    IupSetAttribute(frm_input, "FGCOLOR", CLR_TEXT_NORMAL);

    Ihandle *btn_host = IupButton("创建房间", NULL);
    IupSetCallback(btn_host, "ACTION", (Icallback)btn_network_host_cb);
    IupSetAttribute(btn_host, "SIZE", "100x32");
    IupSetAttribute(btn_host, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_host, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_host, "FLAT", "YES");

    Ihandle *btn_join = IupButton("加入房间", NULL);
    IupSetCallback(btn_join, "ACTION", (Icallback)btn_network_join_cb);
    IupSetAttribute(btn_join, "SIZE", "100x32");
    IupSetAttribute(btn_join, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_join, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_join, "FLAT", "YES");

    Ihandle *btn_cancel = IupButton("取消", NULL);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_network_cancel_cb);
    IupSetAttribute(btn_cancel, "SIZE", "80x32");
    IupSetAttribute(btn_cancel, "BGCOLOR", CLR_BTN_NORMAL_BG);
    IupSetAttribute(btn_cancel, "FGCOLOR", CLR_BTN_NORMAL_FG);
    IupSetAttribute(btn_cancel, "FLAT", "YES");

    Ihandle *hbox_btns = IupHbox(btn_host, btn_join, btn_cancel, NULL);
    IupSetAttribute(hbox_btns, "GAP", "8");
    IupSetAttribute(hbox_btns, "ALIGNMENT", "ACENTER");

    Ihandle *vbox = IupVbox(frm_input, hbox_btns, NULL);
    IupSetAttribute(vbox, "MARGIN", "15x15");
    IupSetAttribute(vbox, "GAP", "12");
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");

    Ihandle *dlg = IupDialog(vbox);
    IupSetAttribute(dlg, "TITLE", "局域网联机");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "BGCOLOR", CLR_WINDOW_BG);

    IupPopup(dlg, IUP_CENTER, IUP_CENTER);
    IupDestroy(dlg);

    return IUP_DEFAULT;
}
// --- 网络对战结束 ---

static int btn_replay_cb(Ihandle *ih)
{
    (void)ih;
    start_replay_gui();
    IupHide(menu_dlg);
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
    if (new_size < MIN_BOARD_SIZE)
        new_size = MIN_BOARD_SIZE;
    if (new_size > MAX_BOARD_SIZE)
        new_size = MAX_BOARD_SIZE;
    BOARD_SIZE = new_size;

    use_forbidden_moves = IupGetInt(tgl_forbidden, "VALUE");

    use_timer = IupGetInt(tgl_timer, "VALUE");
    if (use_timer)
    {
        int minutes = IupGetInt(txt_time_limit, "VALUE");
        if (minutes < 1)
            minutes = 1;
        time_limit = minutes * 60;
    }

    int ai_level = IupGetInt(lst_ai, "VALUE");
    if (ai_level < 1)
        ai_level = 1;
    if (ai_level > 5)
        ai_level = 5;
    ai_difficulty = ai_level;
    defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT + (ai_difficulty - 1) * 0.1;

    // LLM 设置
    Ihandle *lst_ai_mode = IupGetDialogChild(dlg, "AI_MODE");
    Ihandle *txt_endpoint = IupGetDialogChild(dlg, "LLM_ENDPOINT");
    Ihandle *txt_apikey = IupGetDialogChild(dlg, "LLM_API_KEY");
    Ihandle *txt_model = IupGetDialogChild(dlg, "LLM_MODEL");

    int ai_mode = IupGetInt(lst_ai_mode, "VALUE");
    llm_use = (ai_mode == 2) ? 1 : 0;

    char *endpoint = IupGetAttribute(txt_endpoint, "VALUE");
    if (endpoint)
    {
        strncpy(llm_endpoint, endpoint, MAX_LLM_ENDPOINT_LEN - 1);
        llm_endpoint[MAX_LLM_ENDPOINT_LEN - 1] = '\0';
    }

    char *apikey = IupGetAttribute(txt_apikey, "VALUE");
    if (apikey)
    {
        strncpy(llm_api_key, apikey, MAX_LLM_API_KEY_LEN - 1);
        llm_api_key[MAX_LLM_API_KEY_LEN - 1] = '\0';
    }

    char *model = IupGetAttribute(txt_model, "VALUE");
    if (model)
    {
        strncpy(llm_model, model, MAX_LLM_MODEL_LEN - 1);
        llm_model[MAX_LLM_MODEL_LEN - 1] = '\0';
    }

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

    // === 基本设置 ===
    Ihandle *lbl_board_size = IupLabel("棋盘大小:");
    Ihandle *txt_board_size = IupText(NULL);
    IupSetAttribute(txt_board_size, "NAME", "BOARD_SIZE");
    IupSetAttribute(txt_board_size, "SPIN", "YES");
    IupSetAttribute(txt_board_size, "SPINMIN", "5");
    IupSetAttribute(txt_board_size, "SPINMAX", "25");
    IupSetInt(txt_board_size, "VALUE", BOARD_SIZE);
    IupSetAttribute(txt_board_size, "SIZE", "50x");

    Ihandle *hbox_board = IupHbox(lbl_board_size, txt_board_size, NULL);
    IupSetAttribute(hbox_board, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_board, "GAP", "10");

    Ihandle *tgl_forbidden = IupToggle("启用禁手规则", NULL);
    IupSetAttribute(tgl_forbidden, "NAME", "FORBIDDEN");
    IupSetInt(tgl_forbidden, "VALUE", use_forbidden_moves);

    Ihandle *tgl_timer = IupToggle("启用计时器", NULL);
    IupSetAttribute(tgl_timer, "NAME", "TIMER");
    IupSetInt(tgl_timer, "VALUE", use_timer);
    IupSetCallback(tgl_timer, "ACTION", (Icallback)tgl_timer_cb);

    Ihandle *lbl_time_limit = IupLabel("时间限制(分钟):");
    Ihandle *txt_time_limit = IupText(NULL);
    IupSetAttribute(txt_time_limit, "NAME", "TIME_LIMIT");
    IupSetAttribute(txt_time_limit, "SPIN", "YES");
    IupSetAttribute(txt_time_limit, "SPINMIN", "1");
    IupSetAttribute(txt_time_limit, "SPINMAX", "60");
    IupSetInt(txt_time_limit, "VALUE", time_limit / 60);
    IupSetAttribute(txt_time_limit, "ACTIVE", use_timer ? "YES" : "NO");
    IupSetAttribute(txt_time_limit, "SIZE", "50x");

    Ihandle *hbox_time = IupHbox(lbl_time_limit, txt_time_limit, NULL);
    IupSetAttribute(hbox_time, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_time, "GAP", "10");

    Ihandle *vbox_basic = IupVbox(hbox_board, tgl_forbidden, tgl_timer, hbox_time, NULL);
    IupSetAttribute(vbox_basic, "GAP", "8");
    IupSetAttribute(vbox_basic, "MARGIN", "10x8");

    Ihandle *frm_basic = IupFrame(vbox_basic);
    IupSetAttribute(frm_basic, "TITLE", "基本设置");
    IupSetAttribute(frm_basic, "FONT", "SimHei, 11");
    IupSetAttribute(frm_basic, "FGCOLOR", CLR_TEXT_NORMAL);

    // === AI 设置 ===
    Ihandle *lbl_ai = IupLabel("AI 难度:");
    Ihandle *lst_ai = IupList(NULL);
    IupSetAttribute(lst_ai, "NAME", "AI_DIFFICULTY");
    IupSetAttribute(lst_ai, "DROPDOWN", "YES");
    IupSetAttribute(lst_ai, "1", "1 简单");
    IupSetAttribute(lst_ai, "2", "2 普通");
    IupSetAttribute(lst_ai, "3", "3 中等");
    IupSetAttribute(lst_ai, "4", "4 困难");
    IupSetAttribute(lst_ai, "5", "5 专家");
    IupSetInt(lst_ai, "VALUE", ai_difficulty);
    IupSetAttribute(lst_ai, "SIZE", "80x");

    Ihandle *hbox_ai = IupHbox(lbl_ai, lst_ai, NULL);
    IupSetAttribute(hbox_ai, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_ai, "GAP", "10");

    Ihandle *lbl_ai_mode = IupLabel("AI模式:");
    Ihandle *lst_ai_mode = IupList(NULL);
    IupSetAttribute(lst_ai_mode, "NAME", "AI_MODE");
    IupSetAttribute(lst_ai_mode, "DROPDOWN", "YES");
    IupSetAttribute(lst_ai_mode, "1", "算法AI (本地)");
    IupSetAttribute(lst_ai_mode, "2", "大模型AI (在线)");
    IupSetInt(lst_ai_mode, "VALUE", llm_use ? 2 : 1);
    IupSetAttribute(lst_ai_mode, "SIZE", "120x");

    Ihandle *hbox_ai_mode = IupHbox(lbl_ai_mode, lst_ai_mode, NULL);
    IupSetAttribute(hbox_ai_mode, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_ai_mode, "GAP", "10");

    Ihandle *vbox_ai = IupVbox(hbox_ai, hbox_ai_mode, NULL);
    IupSetAttribute(vbox_ai, "GAP", "8");
    IupSetAttribute(vbox_ai, "MARGIN", "10x8");

    Ihandle *frm_ai = IupFrame(vbox_ai);
    IupSetAttribute(frm_ai, "TITLE", "AI 设置");
    IupSetAttribute(frm_ai, "FONT", "SimHei, 11");
    IupSetAttribute(frm_ai, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 大模型设置 ===
    Ihandle *lbl_endpoint = IupLabel("API地址:");
    Ihandle *txt_endpoint = IupText(NULL);
    IupSetAttribute(txt_endpoint, "NAME", "LLM_ENDPOINT");
    IupSetAttribute(txt_endpoint, "VALUE", llm_endpoint);
    IupSetAttribute(txt_endpoint, "SIZE", "250x");

    Ihandle *hbox_endpoint = IupHbox(lbl_endpoint, txt_endpoint, NULL);
    IupSetAttribute(hbox_endpoint, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_endpoint, "GAP", "10");

    Ihandle *lbl_apikey = IupLabel("API Key:");
    Ihandle *txt_apikey = IupText(NULL);
    IupSetAttribute(txt_apikey, "NAME", "LLM_API_KEY");
    IupSetAttribute(txt_apikey, "VALUE", llm_api_key);
    IupSetAttribute(txt_apikey, "PASSWORD", "YES");
    IupSetAttribute(txt_apikey, "SIZE", "200x");

    Ihandle *hbox_apikey = IupHbox(lbl_apikey, txt_apikey, NULL);
    IupSetAttribute(hbox_apikey, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_apikey, "GAP", "10");

    Ihandle *lbl_model = IupLabel("模型名称:");
    Ihandle *txt_model = IupText(NULL);
    IupSetAttribute(txt_model, "NAME", "LLM_MODEL");
    IupSetAttribute(txt_model, "VALUE", llm_model);
    IupSetAttribute(txt_model, "SIZE", "150x");

    Ihandle *hbox_model = IupHbox(lbl_model, txt_model, NULL);
    IupSetAttribute(hbox_model, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_model, "GAP", "10");

    Ihandle *vbox_llm = IupVbox(hbox_endpoint, hbox_apikey, hbox_model, NULL);
    IupSetAttribute(vbox_llm, "GAP", "8");
    IupSetAttribute(vbox_llm, "MARGIN", "10x8");

    Ihandle *frm_llm = IupFrame(vbox_llm);
    IupSetAttribute(frm_llm, "TITLE", "大模型设置");
    IupSetAttribute(frm_llm, "FONT", "SimHei, 11");
    IupSetAttribute(frm_llm, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 按钮 ===
    Ihandle *btn_save = IupButton("保存", NULL);
    IupSetCallback(btn_save, "ACTION", (Icallback)btn_save_settings_cb);
    IupSetAttribute(btn_save, "SIZE", "80x30");
    IupSetAttribute(btn_save, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_save, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_save, "FLAT", "YES");

    Ihandle *btn_cancel = IupButton("取消", NULL);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_cancel_settings_cb);
    IupSetAttribute(btn_cancel, "SIZE", "80x30");
    IupSetAttribute(btn_cancel, "BGCOLOR", CLR_BTN_NORMAL_BG);
    IupSetAttribute(btn_cancel, "FGCOLOR", CLR_BTN_NORMAL_FG);
    IupSetAttribute(btn_cancel, "FLAT", "YES");

    Ihandle *hbox_btns = IupHbox(btn_save, btn_cancel, NULL);
    IupSetAttribute(hbox_btns, "GAP", "15");
    IupSetAttribute(hbox_btns, "MARGIN", "5x0");
    IupSetAttribute(hbox_btns, "ALIGNMENT", "ACENTER");

    // === 主布局 ===
    Ihandle *vbox = IupVbox(frm_basic, frm_ai, frm_llm, hbox_btns, NULL);
    IupSetAttribute(vbox, "GAP", "10");
    IupSetAttribute(vbox, "MARGIN", "15x15");

    Ihandle *dlg = IupDialog(vbox);
    IupSetAttribute(dlg, "TITLE", "游戏设置");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "MINBOX", "NO");
    IupSetAttribute(dlg, "MAXBOX", "NO");
    IupSetAttribute(dlg, "BGCOLOR", CLR_WINDOW_BG);

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

/**
 * @brief 创建主菜单
 */
void create_main_menu()
{
    if (menu_dlg)
        return;

    // === 标题区 ===
    Ihandle *lbl_title = IupLabel("五子棋");
    IupSetAttribute(lbl_title, "FONT", "SimHei, 32");
    IupSetAttribute(lbl_title, "FGCOLOR", CLR_TEXT_TITLE);
    IupSetAttribute(lbl_title, "ALIGNMENT", "ACENTER");

    Ihandle *lbl_subtitle = IupLabel("Gobang");
    IupSetAttribute(lbl_subtitle, "FONT", "SimHei, 14");
    IupSetAttribute(lbl_subtitle, "FGCOLOR", CLR_TEXT_SECONDARY);
    IupSetAttribute(lbl_subtitle, "ALIGNMENT", "ACENTER");

    // === 游戏模式按钮（主按钮样式：深棕底白字）===
    Ihandle *btn_pvp = IupButton("玩家对战", NULL);
    IupSetCallback(btn_pvp, "ACTION", (Icallback)btn_pvp_cb);
    IupSetAttribute(btn_pvp, "SIZE", "150x38");
    IupSetAttribute(btn_pvp, "FONT", "SimHei, 13");
    IupSetAttribute(btn_pvp, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_pvp, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_pvp, "FLAT", "YES");

    Ihandle *btn_pve = IupButton("人机对战", NULL);
    IupSetCallback(btn_pve, "ACTION", (Icallback)btn_pve_cb);
    IupSetAttribute(btn_pve, "SIZE", "150x38");
    IupSetAttribute(btn_pve, "FONT", "SimHei, 13");
    IupSetAttribute(btn_pve, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_pve, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_pve, "FLAT", "YES");

    Ihandle *btn_net = IupButton("局域网联机", NULL);
    IupSetCallback(btn_net, "ACTION", (Icallback)btn_network_cb);
    IupSetAttribute(btn_net, "SIZE", "150x38");
    IupSetAttribute(btn_net, "FONT", "SimHei, 13");
    IupSetAttribute(btn_net, "BGCOLOR", CLR_BTN_PRIMARY_BG);
    IupSetAttribute(btn_net, "FGCOLOR", CLR_BTN_PRIMARY_FG);
    IupSetAttribute(btn_net, "FLAT", "YES");

    Ihandle *vbox_modes = IupVbox(btn_pvp, btn_pve, btn_net, NULL);
    IupSetAttribute(vbox_modes, "GAP", "8");
    IupSetAttribute(vbox_modes, "ALIGNMENT", "ACENTER");
    IupSetAttribute(vbox_modes, "MARGIN", "15x10");

    Ihandle *frm_modes = IupFrame(vbox_modes);
    IupSetAttribute(frm_modes, "TITLE", "选择模式");
    IupSetAttribute(frm_modes, "FONT", "SimHei, 11");
    IupSetAttribute(frm_modes, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 功能按钮（普通按钮样式：浅棕底深棕字）===
    Ihandle *btn_replay = IupButton("复盘回放", NULL);
    IupSetCallback(btn_replay, "ACTION", (Icallback)btn_replay_cb);
    IupSetAttribute(btn_replay, "SIZE", "120x32");
    IupSetAttribute(btn_replay, "FONT", "SimHei, 11");
    IupSetAttribute(btn_replay, "BGCOLOR", CLR_BTN_NORMAL_BG);
    IupSetAttribute(btn_replay, "FGCOLOR", CLR_BTN_NORMAL_FG);
    IupSetAttribute(btn_replay, "FLAT", "YES");

    Ihandle *btn_settings = IupButton("游戏设置", NULL);
    IupSetCallback(btn_settings, "ACTION", (Icallback)btn_settings_cb);
    IupSetAttribute(btn_settings, "SIZE", "120x32");
    IupSetAttribute(btn_settings, "FONT", "SimHei, 11");
    IupSetAttribute(btn_settings, "BGCOLOR", CLR_BTN_NORMAL_BG);
    IupSetAttribute(btn_settings, "FGCOLOR", CLR_BTN_NORMAL_FG);
    IupSetAttribute(btn_settings, "FLAT", "YES");

    Ihandle *btn_exit = IupButton("退出游戏", NULL);
    IupSetCallback(btn_exit, "ACTION", (Icallback)btn_exit_cb);
    IupSetAttribute(btn_exit, "SIZE", "120x32");
    IupSetAttribute(btn_exit, "FONT", "SimHei, 11");
    IupSetAttribute(btn_exit, "BGCOLOR", CLR_BTN_NORMAL_BG);
    IupSetAttribute(btn_exit, "FGCOLOR", CLR_BTN_NORMAL_FG);
    IupSetAttribute(btn_exit, "FLAT", "YES");

    Ihandle *hbox_func = IupHbox(btn_replay, btn_settings, btn_exit, NULL);
    IupSetAttribute(hbox_func, "GAP", "10");
    IupSetAttribute(hbox_func, "ALIGNMENT", "ACENTER");
    IupSetAttribute(hbox_func, "MARGIN", "10x8");

    Ihandle *frm_func = IupFrame(hbox_func);
    IupSetAttribute(frm_func, "TITLE", "功能");
    IupSetAttribute(frm_func, "FONT", "SimHei, 11");
    IupSetAttribute(frm_func, "FGCOLOR", CLR_TEXT_NORMAL);

    // === 主布局 ===
    Ihandle *vbox = IupVbox(
        lbl_title,
        lbl_subtitle,
        IupLabel(NULL), // 间隔
        frm_modes,
        frm_func,
        NULL);
    IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(vbox, "GAP", "8");
    IupSetAttribute(vbox, "MARGIN", "30x25");

    menu_dlg = IupDialog(vbox);
    IupSetAttribute(menu_dlg, "TITLE", "五子棋 - 主菜单");
    IupSetAttribute(menu_dlg, "RESIZE", "NO");
    IupSetAttribute(menu_dlg, "MINBOX", "NO");
    IupSetAttribute(menu_dlg, "MAXBOX", "NO");
    IupSetAttribute(menu_dlg, "BGCOLOR", CLR_WINDOW_BG);
    IupSetAttribute(menu_dlg, "SIZE", "380x520");

    // 设置对话框关闭回调 (点X关闭程序)
    IupSetCallback(menu_dlg, "CLOSE_CB", (Icallback)btn_exit_cb);
    IupMap(menu_dlg); // Map immediately
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