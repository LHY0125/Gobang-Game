/**
 * @file globals.c
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 全局变量定义和初始化文件
 * @version 6.0
 * @date 2025-07-10
 * @note 集中管理所有全局变量的定义和初始化，提高代码可维护性
 */

#include "globals.h"
#include "config.h"

// ==================== 游戏核心变量定义 ====================
int BOARD_SIZE = DEFAULT_BOARD_SIZE;                           // 实际使用的棋盘尺寸
int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE] = {0};               // 棋盘状态存储数组(默认棋盘全空为0)
Step steps[MAX_STEPS];                                         // 存储所有落子步骤的数组
const int direction[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 四个方向：向下、向右、右下、左下
int step_count = 0;                                            // 当前步数计数器

// ==================== 游戏配置变量定义 ====================
bool use_forbidden_moves = DEFAULT_USE_FORBIDDEN_MOVES;        // 是否启用禁手规则
int use_timer = DEFAULT_USE_TIMER;                             // 是否启用计时器
int time_limit = DEFAULT_TIME_LIMIT;                           // 每回合的时间限制（秒）
int network_port = DEFAULT_NETWORK_PORT;                       // 网络端口
int network_timeout = NETWORK_TIMEOUT_MS;                      // 网络超时时间

// ==================== AI相关变量定义 ====================
double defense_coefficient = DEFAULT_DEFENSE_COEFFICIENT;      // 防守系数

// ==================== 网络相关变量定义 ====================
NetworkGameState network_state = {0};                          // 网络游戏状态

// ==================== 记录相关变量定义 ====================
int player1_final_score = 0;                                  // 玩家1最终得分
int player2_final_score = 0;                                  // 玩家2最终得分
int scores_calculated = 0;                                     // 评分计算标志
char winner_info[50] = "平局或未完成";                          // 存储胜负信息