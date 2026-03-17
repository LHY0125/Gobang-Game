#ifndef GUI_MENU_H
#define GUI_MENU_H

#include <iup.h>

// 主菜单对话框句柄，需要暴露给其他模块（如返回菜单时）
extern Ihandle *menu_dlg;

/**
 * @brief 创建并显示主菜单
 */
void create_main_menu();

/**
 * @brief 显示主菜单
 */
void show_main_menu();

/**
 * @brief 隐藏主菜单
 */
void hide_main_menu();

#endif // GUI_MENU_H
