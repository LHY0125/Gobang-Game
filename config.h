/**
 * @file config.h
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋游戏参数配置头文件
 * @version 5.0
 * @date 2025-07-10
 *
 * @copyright Copyright (c) 2025
 *
 * @note 本文件集中定义了五子棋游戏的所有参数配置，便于统一管理和修改
 */

#ifndef CONFIG_H
#define CONFIG_H

//---------- 棋盘相关参数 ----------//
#define MAX_BOARD_SIZE 25                           // 支持的最大棋盘尺寸
#define MIN_BOARD_SIZE 5                            // 支持的最小棋盘尺寸
#define DEFAULT_BOARD_SIZE 15                       // 默认棋盘尺寸
#define MAX_STEPS (MAX_BOARD_SIZE * MAX_BOARD_SIZE) // 游戏最大步数

//---------- 玩家标识参数 ----------//
#define EMPTY 0                                     // 棋盘空位标识
#define PLAYER 1                                    // 玩家标识 (用于人机对战模式)
#define AI 2                                        // AI标识 (用于人机对战模式)
#define PLAYER1 1                                   // 玩家1标识 (用于双人对战模式)
#define PLAYER2 2                                   // 玩家2标识 (用于双人对战模式)

//---------- 特殊输入命令 ----------//
#define INPUT_UNDO -1       // 悔棋
#define INPUT_SAVE -2       // 保存
#define INPUT_EXIT -3       // 退出
#define INPUT_SURRENDER -4  // 认输

//---------- 游戏设置默认值 ----------//
#define DEFAULT_USE_FORBIDDEN_MOVES false  // 默认不启用禁手规则
#define DEFAULT_USE_TIMER 0                // 默认不启用计时器
#define DEFAULT_TIME_LIMIT 30              // 默认时间限制为30秒(内部存储)

//---------- AI参数 ----------//
#define DEFAULT_AI_DEPTH 3                 // 默认AI搜索深度
#define DEFAULT_DEFENSE_COEFFICIENT 1.2    // 默认防守系数

//---------- 评分参数 ----------//
// 棋型评分 - 用于calculate_step_score函数
#define SCORE_FIVE 0            // 五连
#define SCORE_LIVE_FOUR 2000    // 活四
#define SCORE_RUSH_FOUR 1000    // 冲四
#define SCORE_DEAD_FOUR 300     // 死四
#define SCORE_LIVE_THREE 500    // 活三
#define SCORE_SLEEP_THREE 200   // 眠三
#define SCORE_DEAD_THREE 80     // 死三
#define SCORE_LIVE_TWO 100      // 活二
#define SCORE_SLEEP_TWO 40      // 眠二
#define SCORE_DEAD_TWO 15       // 死二
#define SCORE_LIVE_ONE 15       // 开放单子
#define SCORE_HALF_ONE 8        // 半开放单子
#define SCORE_DEAD_ONE 2        // 封闭单子

// 位置奖励系数
#define POSITION_BONUS_FACTOR 10     // 位置奖励因子

// AI评估参数 - 用于evaluate_pos函数
#define AI_SCORE_FIVE 1000000        // AI评估-五连
#define AI_SCORE_LIVE_FOUR 100000    // AI评估-活四
#define AI_SCORE_RUSH_FOUR 10000     // AI评估-冲四
#define AI_SCORE_DEAD_FOUR 500       // AI评估-死四
#define AI_SCORE_LIVE_THREE 5000     // AI评估-活三
#define AI_SCORE_SLEEP_THREE 1000    // AI评估-眠三
#define AI_SCORE_DEAD_THREE 50       // AI评估-死三
#define AI_SCORE_LIVE_TWO 500        // AI评估-活二
#define AI_SCORE_SLEEP_TWO 100       // AI评估-眠二
#define AI_SCORE_DEAD_TWO 10         // AI评估-死二
#define AI_SCORE_LIVE_ONE 50         // AI评估-开放单子
#define AI_SCORE_HALF_ONE 10         // AI评估-半开放单子
#define AI_SCORE_DEAD_ONE 1          // AI评估-封闭单子

// AI位置奖励系数
#define AI_POSITION_BONUS_FACTOR 50  // AI位置奖励因子

// 搜索算法参数
#define SEARCH_MAX_SCORE 1000000     // 搜索最大分数
#define SEARCH_WIN_BONUS 1000000     // 获胜奖励分数
#define AI_NEARBY_RANGE 2            // AI搜索的邻近范围
#define AI_SEARCH_RANGE_THRESHOLD 10 // AI开始限制搜索范围的步数阈值

// 评分权重参数
#define TIME_WEIGHT_FACTOR 0.5       // 时间权重因子
#define WIN_BONUS 2000               // 胜利奖励分数

// 文件路径参数
#define RECORDS_DIR "records"        // 记录文件目录
#define MAX_PATH_LENGTH 256          // 最大路径长度

//---------- 配置管理函数声明 ----------//
/**
 * @brief 加载游戏配置
 */
void load_game_config();

/**
 * @brief 保存游戏配置
 */
void save_game_config();

/**
 * @brief 重置为默认配置
 */
void reset_to_default_config();

/**
 * @brief 显示当前配置
 */
void display_current_config();

/**
 * @brief 配置棋盘大小
 */
void config_board_size();

/**
 * @brief 配置禁手规则
 */
void config_forbidden_moves();

/**
 * @brief 配置计时器
 */
void config_timer();

/**
 * @brief 配置管理主菜单
 */
void config_management_menu();

#endif // CONFIG_H