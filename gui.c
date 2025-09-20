/**
 * @file gui.c
 * @brief 图形化用户界面实现文件
 * @note 使用SDL3库实现五子棋的图形化界面
 * @author 刘航宇
 * @date 2025-01-15
 */

#include "gui.h"
#include "ui.h"
#include "globals.h"
#include "game_mode.h"
#include "init_board.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief 初始化GUI
 * @details 初始化SDL3图形库和游戏界面组件：
 *          - 初始化SDL视频子系统
 *          - 创建游戏窗口（可调整大小）
 *          - 创建SDL渲染器
 *          - 初始化游戏状态和棋盘数据
 * @return 成功返回0，失败返回-1
 * @note 窗口标题为"五子棋游戏 - SDL3版本"
 *       窗口尺寸由WINDOW_WIDTH和WINDOW_HEIGHT定义
 *       失败时会自动清理已创建的资源
 */
int init_gui() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow(
        "五子棋游戏 - SDL3版本",
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    
    // 设置窗口位置到屏幕中央
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    
    if (!window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 显示窗口
    SDL_ShowWindow(window);

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 初始化游戏状态
    // 初始化棋盘
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = EMPTY;
        }
    }
    current_player_gui = PLAYER;
    game_over = 0;
    
    printf("图形化界面初始化成功！\n");
    printf("使用鼠标点击棋盘进行落子\n");
    printf("按ESC键退出游戏\n");
    
    return 0;
}

/**
 * @brief 清理GUI资源
 * @details 按顺序释放所有SDL相关资源：
 *          - 销毁SDL渲染器
 *          - 销毁SDL窗口
 *          - 退出SDL子系统
 * @note 函数会检查资源是否存在再进行释放
 *       释放后将指针设置为NULL防止重复释放
 *       程序退出时必须调用此函数避免内存泄漏
 */
void cleanup_gui() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
    printf("图形化界面已关闭\n");
}

/**
 * @brief 渲染游戏画面
 * @details 完整的游戏画面渲染流程：
 *          - 清空屏幕并设置背景色
 *          - 绘制棋盘网格和标记点
 *          - 绘制所有棋子
 *          - 绘制UI界面元素
 *          - 将渲染结果显示到屏幕
 * @note 使用双缓冲技术，通过SDL_RenderPresent显示最终结果
 *       背景色由GUI_COLOR_BACKGROUND定义
 *       每帧都会完全重绘整个画面
 */
void render_game() {
    // 清空屏幕 - 设置背景色
    SDL_Color bg_color = GUI_COLOR_BACKGROUND;
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(renderer);
    
    // 绘制棋盘
    draw_board();
    
    // 绘制棋子
    draw_stones();
    
    // 绘制UI元素
    draw_ui_elements();
    
    // 显示渲染结果
    SDL_RenderPresent(renderer);
}

/**
 * @brief 处理事件
 * @details 处理所有SDL事件并执行相应操作：
 *          - SDL_EVENT_QUIT：用户关闭窗口
 *          - SDL_EVENT_KEY_DOWN：键盘按键（ESC退出）
 *          - SDL_EVENT_MOUSE_BUTTON_DOWN：鼠标点击落子
 * @return 继续运行返回1，退出返回0
 * @note 鼠标左键点击会转换为棋盘坐标并尝试落子
 *       落子后会检查胜负并切换玩家
 *       游戏结束后不再响应落子操作
 */
int handle_events() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                gui_running = 0;
                return 0;
                
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    gui_running = 0;
                    return 0;
                }
                break;
                
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT && !game_over) {
                    int board_x, board_y;
                    if (screen_to_board(event.button.x, event.button.y, &board_x, &board_y)) {
                        if (have_space(board_x, board_y)) {
                            // 执行落子操作
                            if (player_move(board_x, board_y, current_player_gui)) {
                                // 检查是否获胜
                                if (check_win(board_x, board_y, current_player_gui)) {
                                    game_over = 1;
                                    if (current_player_gui == PLAYER) {
                                        sprintf(status_message, "游戏结束 - 黑子获胜！");
                                    } else {
                                        sprintf(status_message, "游戏结束 - 白子获胜！");
                                    }
                                } else {
                                    // 切换玩家
                                    current_player_gui = (current_player_gui == PLAYER) ? AI : PLAYER;
                                    if (current_player_gui == PLAYER) {
                                        sprintf(status_message, "轮到黑子下棋");
                                    } else {
                                        sprintf(status_message, "轮到白子下棋");
                                    }
                                }
                            }
                        } else {
                            sprintf(status_message, "该位置已有棋子，请选择其他位置");
                        }
                    }
                }
                break;
        }
    }
    return 1;
}

/**
 * @brief 绘制棋盘
 * @details 绘制15x15的五子棋棋盘，包括：
 *          - 横竖交叉的网格线
 *          - 天元点（棋盘中心的标记点）
 *          - 四个星位（棋盘上的定位点）
 * @note 使用SDL3渲染器绘制线条和填充矩形
 *       棋盘线条颜色由GUI_COLOR_BOARD_LINE定义
 *       天元点和星位用黑色小矩形标记
 */
void draw_board() {
    SDL_Color line_color = GUI_COLOR_BOARD_LINE;
    SDL_SetRenderDrawColor(renderer, line_color.r, line_color.g, line_color.b, line_color.a);
    
    // 绘制横线
    for (int i = 0; i < BOARD_SIZE; i++) {
        int y = BOARD_OFFSET_Y + i * CELL_SIZE;
        SDL_RenderLine(renderer, 
            BOARD_OFFSET_X, y,
            BOARD_OFFSET_X + (BOARD_SIZE - 1) * CELL_SIZE, y);
    }
    
    // 绘制竖线
    for (int j = 0; j < BOARD_SIZE; j++) {
        int x = BOARD_OFFSET_X + j * CELL_SIZE;
        SDL_RenderLine(renderer,
            x, BOARD_OFFSET_Y,
            x, BOARD_OFFSET_Y + (BOARD_SIZE - 1) * CELL_SIZE);
    }
    
    // 绘制天元点和星位
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    int center = BOARD_SIZE / 2;
    
    // 天元点
    int center_x = BOARD_OFFSET_X + center * CELL_SIZE;
    int center_y = BOARD_OFFSET_Y + center * CELL_SIZE;
    SDL_FRect center_rect = {center_x - 2, center_y - 2, 4, 4};
    SDL_RenderFillRect(renderer, &center_rect);
    
    // 四个星位
    int star_offset = 3;
    int positions[][2] = {
        {center - star_offset, center - star_offset},
        {center + star_offset, center - star_offset},
        {center - star_offset, center + star_offset},
        {center + star_offset, center + star_offset}
    };
    
    for (int i = 0; i < 4; i++) {
        int x = BOARD_OFFSET_X + positions[i][1] * CELL_SIZE;
        int y = BOARD_OFFSET_Y + positions[i][0] * CELL_SIZE;
        SDL_FRect star_rect = {x - 1, y - 1, 2, 2};
        SDL_RenderFillRect(renderer, &star_rect);
    }
}

/**
 * @brief 绘制棋子
 * @details 遍历整个棋盘数组，绘制所有已落下的棋子：
 *          - 黑子：使用GUI_COLOR_BLACK_STONE颜色
 *          - 白子：使用GUI_COLOR_WHITE_STONE颜色
 *          - 每个棋子都有边框：使用GUI_COLOR_STONE_BORDER颜色
 * @note 棋子绘制为圆形，半径由STONE_RADIUS定义
 *       通过draw_circle函数实现圆形绘制
 *       棋子位置根据棋盘坐标和CELL_SIZE计算屏幕坐标
 */
void draw_stones() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != EMPTY) {
                int x = BOARD_OFFSET_X + j * CELL_SIZE;
                int y = BOARD_OFFSET_Y + i * CELL_SIZE;
                
                // 设置棋子颜色
                SDL_Color stone_color, border_color;
                 if (board[i][j] == PLAYER || board[i][j] == PLAYER1) {
                     stone_color = (SDL_Color)GUI_COLOR_BLACK_STONE;
                 } else {
                     stone_color = (SDL_Color)GUI_COLOR_WHITE_STONE;
                 }
                 border_color = (SDL_Color)GUI_COLOR_STONE_BORDER;
                
                // 绘制圆形棋子
                draw_circle(x, y, STONE_RADIUS, stone_color);
                draw_circle(x, y, STONE_RADIUS, border_color);
                
                // 重新绘制内部
                draw_circle(x, y, STONE_RADIUS - 1, stone_color);
            }
        }
    }
}

/**
 * @brief 绘制圆形
 * @param center_x 圆心X坐标
 * @param center_y 圆心Y坐标
 * @param radius 半径
 * @param color 颜色
 * @details 使用像素级绘制实现圆形：
 *          - 遍历圆形外接矩形内的所有像素点
 *          - 计算每个像素到圆心的距离
 *          - 距离小于等于半径的像素点进行着色
 * @note 采用暴力算法，性能较低但实现简单
 *       适用于绘制棋子等小尺寸圆形
 *       SDL3没有内置圆形绘制函数，需要自实现
 */
void draw_circle(int center_x, int center_y, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
}

/**
 * @brief 绘制UI元素
 */
void draw_ui_elements() {
    // 绘制状态信息区域背景
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_FRect info_rect = {BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE + 20, BOARD_OFFSET_Y, 200, 100};
    SDL_RenderFillRect(renderer, &info_rect);
    
    // 绘制边框
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &info_rect);
    
    // 这里可以添加文字渲染，但SDL3需要额外的字体库
    // 暂时用简单的图形表示当前玩家
    int indicator_x = info_rect.x + 20;
    int indicator_y = info_rect.y + 20;
    
    if (!game_over) {
        if (current_player_gui == PLAYER) {
            // 黑子回合
            draw_circle(indicator_x, indicator_y, 10, (SDL_Color){0, 0, 0, 255});
        } else {
            // 白子回合
            draw_circle(indicator_x, indicator_y, 10, (SDL_Color){255, 255, 255, 255});
            // 绘制当前玩家指示器（简单的矩形代替圆形）
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect indicator_rect = {indicator_x - 10, indicator_y - 10, 20, 20};
            SDL_RenderFillRect(renderer, &indicator_rect);
        }
    }
}

/**
 * @brief 屏幕坐标转棋盘坐标
 * @param screen_x 屏幕X坐标
 * @param screen_y 屏幕Y坐标
 * @param board_x 输出棋盘X坐标
 * @param board_y 输出棋盘Y坐标
 * @return 转换成功返回1，失败返回0
 * @details 坐标转换算法：
 *          - 减去棋盘偏移量得到相对坐标
 *          - 加上半个格子尺寸实现就近取整
 *          - 除以格子尺寸得到棋盘坐标
 *          - 检查坐标是否在有效范围内
 * @note 使用就近取整算法，点击格子中心附近都会定位到该格子
 *       坐标范围检查确保不会越界访问棋盘数组
 */
int screen_to_board(int screen_x, int screen_y, int* board_x, int* board_y) {
    int rel_x = screen_x - BOARD_OFFSET_X;
    int rel_y = screen_y - BOARD_OFFSET_Y;
    
    *board_x = (rel_x + CELL_SIZE/2) / CELL_SIZE;
    *board_y = (rel_y + CELL_SIZE/2) / CELL_SIZE;
    
    return (*board_x >= 0 && *board_x < BOARD_SIZE && 
            *board_y >= 0 && *board_y < BOARD_SIZE);
}

/**
 * @brief 显示消息
 * @param message 要显示的消息
 * @details 消息显示功能：
 *          - 将消息复制到全局状态消息缓冲区
 *          - 同时在控制台输出消息内容
 *          - 确保字符串安全复制，防止缓冲区溢出
 * @note 消息会存储在status_message全局变量中
 *       字符串长度限制为缓冲区大小减1
 *       消息可用于游戏状态提示和错误信息显示
 */
void show_message(const char* message) {
    strncpy(status_message, message, sizeof(status_message) - 1);
    status_message[sizeof(status_message) - 1] = '\0';
    printf("%s\n", message);
}