/**
 * @file gobang.h
 * @brief 五子棋游戏核心逻辑头文件
 * @details 定义了游戏的核心数据结构、全局变量、宏定义和函数原型。
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @date 2025-07-02
 * @version 4.0
 * @note
 * 1. 新增功能：
 *    - 增加了对禁手规则的支持，防止玩家进行无意义的走法。
 *    - 新增了游戏计时器功能，限制每回合的思考时间。
 * 2. 性能优化：
 *    - 优化了评估函数的性能，减少了不必要的计算。
 *    - 引入了 Alpha-Beta 剪枝算法，提高了 AI 搜索的效率。
 * 3. 用户界面改进：
 *    - 新增了命令行界面，提供更友好的交互体验。
 *    - 可以自定义棋盘大小，增加游戏的灵活性。
 * 4. 代码结构优化：
 *    - 将游戏逻辑和用户界面分离，提高代码的可读性和可维护性。
 *    - 优化了代码结构，提高了代码的可读性和可维护性。
 * 5. 异常处理：
 *    - 增加了输入错误的异常处理机制，确保游戏的稳定性。
 *    - 修复了一些已知的 bug，提高游戏的稳定性。
 * 6. 文档更新：
 *    - 完善了代码注释，提高了代码的可读性。
 *    - 更新了文档，包括功能描述、使用方法、注意事项等。
 * 7. 版本控制：
 *    - 使用 Git 进行版本控制，方便团队协作和代码管理。
 * 8. 测试：
 *    - 进行了全面的测试，确保游戏的稳定性和功能的正确性。
 * 9. 开源协议：
 *    - 选择了 MIT 开源协议，允许用户自由使用、修改和分发代码。
 * 10. 贡献者：
 *    - 刘航宇
 * 11. 联系信息：
 *    - 项目主页：[https://github.com/LHY0125/Gobang-Game]
 *    - 联系邮箱：[3364451258@qq.com][15236416560@163.com][lhy3364451258@outlook.com]
 */

#ifndef GO_BANG_H
#define GO_BANG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// 宏定义
#define MAX_BOARD_SIZE 25                           // 支持的最大棋盘尺寸
#define PLAYER 1                                    // 玩家标识 (用于人机对战模式)
#define AI 2                                        // AI标识 (用于人机对战模式)
#define PLAYER1 1                                   // 玩家1标识 (用于双人对战模式)
#define PLAYER2 2                                   // 玩家2标识 (用于双人对战模式)
#define EMPTY 0                                     // 棋盘空位标识
#define MAX_STEPS (MAX_BOARD_SIZE * MAX_BOARD_SIZE) // 游戏最大步数

// 全局变量
extern int BOARD_SIZE;                              // 当前实际使用的棋盘尺寸
extern int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];   // 存储棋盘状态的二维数组
extern int step_count;                              // 当前游戏的总步数
extern bool use_forbidden_moves;                    // 是否启用禁手规则的标志
extern int use_timer;                               // 是否启用计时器的标志
extern int time_limit;                              // 每回合的时间限制（秒）
extern const int direction[4][2];                   // 定义四个基本搜索方向：水平、垂直、左斜、右斜

// 数据结构
/**
 * @brief 记录一步棋的详细信息
 */
typedef struct
{
    int player; // 执行该步的玩家标识
    int x;      // 落子的行坐标 (0-based)
    int y;      // 落子的列坐标 (0-based)
} Step;

extern Step steps[MAX_STEPS]; // 用于存储游戏中每一步棋的数组

/**
 * @brief 存储在特定方向上棋子连续性的信息
 * @details 用于评估棋形，例如判断活三、冲四等关键形态。
 */
typedef struct
{
    int continuous_chess; // 连续同色棋子的数量
    bool check_start;     // 棋子序列的起始端是否为空位（即是否开放）
    bool check_end;       // 棋子序列的末尾端是否为空位（即是否开放）
} DirInfo;

// 函数原型


// --- 游戏核心逻辑 ---
/**
 * @brief 检查指定坐标是否为有效落子点（在棋盘内且为空）
 * @param x 待检查的行坐标 (0-based)
 * @param y 待检查的列坐标 (0-based)
 * @return 若位置有效且为空则返回true，否则返回false
 */
bool have_space(int x, int y);

/**
 * @brief 判断一个落子是否为禁手
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 如果是禁手则返回true，否则返回false
 */
bool is_forbidden_move(int x, int y, int player);


/**
 * @brief 执行一次玩家落子操作
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 若落子成功则返回true，否则（位置无效或被占用）返回false
 */
bool player_move(int x, int y, int player);

/**
 * @brief 计算在特定方向上的棋子连续信息
 * @param x 起始点的行坐标
 * @param y 起始点的列坐标
 * @param dx x方向的增量 (-1, 0, or 1)
 * @param dy y方向的增量 (-1, 0, or 1)
 * @param player 玩家标识
 * @return 返回一个包含连续棋子信息的 DirInfo 结构体
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player);

/**
 * @brief 检查在某点落子后，该玩家是否获胜
 * @param x 落子的行坐标 (0-based)
 * @param y 落子的列坐标 (0-based)
 * @param player 当前玩家的标识
 * @return 如果获胜则返回true，否则返回false
 */
bool check_win(int x, int y, int player);

/**
 * @brief 悔棋功能，撤销指定步数
 * @param steps_to_undo 要撤销的步数（每步包含双方各一次落子）
 * @return 若悔棋成功则返回true，否则返回false
 */
bool return_move(int steps_to_undo);

/**
 * @brief 计算并返回一步棋的得分
 * @param x 落子的行坐标
 * @param y 落子的列坐标
 * @param player 玩家标识
 * @return 该步棋的得分
 */
int calculate_step_score(int x, int y, int player);

#endif // GO_BANG_H