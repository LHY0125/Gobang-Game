/**
 * @file globals.h
 * @brief 全局变量声明头文件
 * @note 集中管理所有全局变量的声明，提高代码可维护性
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "type.h"
#include "gobang.h"
#include "network.h"
#include <stdbool.h>

// ==================== 游戏核心变量 ====================
extern int BOARD_SIZE;                              // 当前实际使用的棋盘尺寸
extern int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];   // 棋盘状态存储数组
extern Step steps[MAX_STEPS];                       // 存储所有落子步骤的数组
extern const int direction[4][2];                   // 四个方向：向下、向右、右下、左下
extern int step_count;                              // 当前步数计数器

// ==================== 游戏配置变量 ====================
extern bool use_forbidden_moves;                    // 是否启用禁手规则的标志
extern int use_timer;                               // 是否启用计时器的标志
extern int time_limit;                              // 每回合的时间限制（秒，内部存储）
extern int network_port;                            // 网络端口
extern int network_timeout;                         // 网络超时时间

// ==================== AI相关变量 ====================
extern double defense_coefficient;                  // 防守系数

// ==================== 网络相关变量 ====================
extern NetworkGameState network_state;              // 网络游戏状态

// ==================== 记录相关变量 ====================
extern int player1_final_score;                     // 玩家1最终得分
extern int player2_final_score;                     // 玩家2最终得分
extern int scores_calculated;                       // 评分计算标志
extern char winner_info[50];                        // 存储胜负信息

#endif // GLOBALS_H