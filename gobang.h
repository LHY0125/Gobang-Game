/**
 * @file gobang.h
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋游戏头文件
 * @version 3.0
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 *
 * @note 本文件为gobang.c的接口文件，提供游戏所需的所有函数声明
 * @note 设计要点：
 * 1. 支持5x5到25x25的可变棋盘尺寸
 * 2. 使用极小极大算法实现AI决策
 * 3. 提供完整的游戏过程复盘功能
 * 4. 所有坐标采用0-base索引
 * 5. 使用全局变量简化状态管理
 * 6. 实现了非阻塞的超时检测，超时后能立刻结束回合
 * 7. 计时单位为分钟，方便用户设置
 */

#ifndef GO_BANG_H
#define GO_BANG_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

// 宏定义
/**
 * @brief 最大支持棋盘尺寸
 * @note 25x25是性能与实用性的平衡点，更大的棋盘会显著降低AI响应速度
 */
#define MAX_BOARD_SIZE 25 // 最大支持棋盘尺寸(5x5到25x25)

/**
 * @brief 玩家标识符
 * @note 使用1/2而非字符标识，便于扩展为多玩家游戏
 */
#define PLAYER 1  // 玩家棋子标识符
#define AI 2      // AI棋子标识符
#define PLAYER3 3 // 玩家3棋子标识符
#define PLAYER4 4 // 玩家4棋子标识符

/**
 * @brief 空位置标识符
 * @note 必须与PLAYER/AI的值不同
 */
#define EMPTY 0 // 空位置标识符

/**
 * @brief 最大步数限制
 * @note 等于棋盘总格数，确保不会数组越界
 */
#define MAX_STEPS (MAX_BOARD_SIZE * MAX_BOARD_SIZE) // 最大步数（棋盘总格数）

// 全局变量声明
extern int BOARD_SIZE;                            // 实际使用的棋盘尺寸(默认15)
extern int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]; // 棋盘状态存储数组
extern int step_count;                            // 当前步数计数器
extern bool use_forbidden_moves;                  // 是否启用禁手规则
extern int use_timer;                             // 是否启用计时器
extern int time_limit;                            // 时间限制（秒）

/**
 * @brief 落子步骤记录结构体
 * @note 用于存储游戏历史记录和AI决策树
 * @note 字段说明:
 * - player: 标识落子方(PLAYER/AI)
 * - x/y: 0-based坐标位置
 */
typedef struct
{
    int player; // 落子方标识
    int x, y;   // 坐标位置
} Step;

extern Step steps[MAX_STEPS]; // 存储所有落子步骤的数组

bool handle_player_turn(int current_player);

/**
 * @brief 连子检测信息结构体
 * @note 用于五子连珠判断和棋局评估
 * @note 字段说明:
 * - continuous_chess: 连续同色棋子数量
 * - check_start: 序列起点方向是否有空位(可发展性)
 * - check_end: 序列终点方向是否有空位(可发展性)
 */
typedef struct
{
    int continuous_chess; // 连续棋子数量
    bool check_start;     // 序列起点方向是否开放（空位）
    bool check_end;       // 序列终点方向是否开放（空位）
} DirInfo;

// 函数声明
/**
 * @brief 初始化棋盘为全空状态
 */
void empty_board();

/**
 * @brief 打印当前棋盘状态
 */
void print_board();

/**
 * @brief 检查指定位置是否为空且有效
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 位置有效且为空
 * @return false 位置无效或已被占用
 */
bool have_space(int x, int y);

/**
 * @brief 设置棋盘大小
 */
void setup_board_size();

/**
 * @brief 设置游戏选项
 * @note 包括禁手规则、计时器和时间限制
 */
void setup_game_options();

/**
 * @brief 决定先手玩家
 * @param player1 玩家1的标识
 * @param player2 玩家2的标识
 * @return int 先手玩家的标识
 */
int determine_first_player(int player1, int player2);

/**
 * @brief 检查是否为禁手
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识
 * @return true 是禁手
 * @return false 不是禁手
 */
bool is_forbidden_move(int x, int y, int player);

/**
 * @brief 玩家落子操作
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 落子成功
 * @return false 落子失败(位置无效)
 */
bool player_move(int x, int y, int player);

/**
 * @brief 计算特定方向上连续同色棋子数量
 * @param x 起始行坐标
 * @param y 起始列坐标
 * @param dx 行方向增量(-1,0,1)
 * @param dy 列方向增量(-1,0,1)
 * @param player 玩家标识(PLAYER/AI)
 * @return DirInfo 包含连续棋子数和方向开放状态的结构体
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player);

/**
 * @brief 检查特定位置落子后是否形成五连珠
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return true 形成五连珠
 * @return false 未形成五连珠
 */
bool check_win(int x, int y, int player);

/**
 * @brief 评估特定位置对当前玩家的价值
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return int 位置评估分数(越高越好)
 */
int evaluate_pos(int x, int y, int player);

/**
 * @brief 带α-β剪枝的深度优先搜索(极小极大算法)
 * @param x 当前行坐标
 * @param y 当前列坐标
 * @param player 当前玩家
 * @param depth 搜索深度
 * @param alpha α值(当前最大值)
 * @param beta β值(当前最小值)
 * @param is_maximizing 是否为极大化玩家
 * @return int 最佳评估分数
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing);

/**
 * @brief AI落子决策函数
 * 使用评估函数和搜索算法选择最佳落子位置
 */
void ai_move(int depth);

/**
 * @brief 悔棋功能实现
 * @return true 悔棋成功
 * @return false 悔棋失败(步数不足)
 * @note 会撤销玩家和AI的最后一步操作
 */
bool return_move(int steps_to_undo);

/**
 * @brief 复盘游戏过程
 * 逐步重现游戏中的所有落子步骤
 */
void review_process(int game_mode);

/**
 * @brief 评估玩家在整盘棋局中的表现
 * @param player 要评估的玩家(PLAYER/AI)
 * @return int 总分(已考虑方向重复计算)
 */
int evaluate_performance(int player);

/**
 * @brief 计算一步棋的得分
 * @param x 行坐标
 * @param y 列坐标
 * @param player 玩家标识
 * @return int 一步棋的得分
 */
int calculate_step_score(int x, int y, int player);


/**
 * @brief 将当前游戏记录保存到文件
 * @param filename 要保存的文件名
 * @return int 错误码:
 *   0: 成功
 *   1: 目录创建失败
 *   2: 文件打开失败
 *   3: 文件写入失败
 */
int save_game_to_file(const char *filename, int game_mode);

/**
 * @brief 处理游戏结束后的记录保存
 */
void handle_save_record(int game_mode);

/**
 * @brief 从文件加载游戏记录
 * @param filename 要加载的文件名
 * @return true 加载成功
 * @return false 加载失败
 */
int load_game_from_file(const char *filename);

#endif // GO_BANG_H