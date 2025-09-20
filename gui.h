/**
 * @file gui.h
 * @brief 图形化用户界面头文件
 * @note 使用SDL3库实现五子棋的图形化界面
 * @author 刘航宇
 * @date 2025-01-15
 */

#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include "gobang.h"
#include "config.h"
#include "globals.h"

// GUI函数声明

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
int init_gui();

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
void cleanup_gui();

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
void render_game();

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
int handle_events();

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
void draw_board();

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
void draw_stones();

/**
 * @brief 绘制UI元素
 * @details 绘制游戏界面的用户交互元素：
 *          - 状态信息区域背景和边框
 *          - 当前玩家指示器（黑子或白子圆形）
 *          - 游戏状态显示区域
 * @note 暂时使用简单图形代替文字显示
 *       需要额外字体库支持文字渲染
 *       指示器位置在棋盘右侧固定区域
 */
void draw_ui_elements();

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
void draw_circle(int center_x, int center_y, int radius, SDL_Color color);

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
int screen_to_board(int screen_x, int screen_y, int *board_x, int *board_y);

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
void show_message(const char *message);

#endif // GUI_H