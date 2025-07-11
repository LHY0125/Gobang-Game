/**
 * @file record.h
 * @brief 游戏复盘与记录头文件
 * @note 本文件定义了游戏复盘与记录相关的函数和数据结构。
 * 它负责管理游戏的历史记录、加载和保存游戏文件、计算游戏评分等功能。
 */
#ifndef RECORD_H
#define RECORD_H

#include "gobang.h"

// --- 复盘与记录功能 ---
/**
 * @brief 进入复盘流程，回顾整局游戏
 * @param game_mode 游戏模式（1为人机对战，2为双人对战）
 */
void review_process(int game_mode);

/**
 * @brief 将当前对局记录保存到文件
 * @param filename 要保存到的文件名
 * @param game_mode 游戏模式
 * @return 0表示成功，非0表示失败
 */
int save_game_to_file(const char *filename, int game_mode);

/**
 * @brief 处理保存游戏记录的逻辑
 * @param game_mode 游戏模式
 */
void handle_save_record(int game_mode);

/**
 * @brief 从文件加载游戏记录
 * @param filename 要加载的文件名
 * @return 游戏模式（1或2），0表示失败
 */
int load_game_from_file(const char *filename);

/**
 * @brief 计算游戏评分
 */
void calculate_game_scores();

/**
 * @brief 显示游戏评分结果和MVP评选
 * @param game_mode 游戏模式（1-人机对战，2-双人对战）
 */
void display_game_scores(int game_mode);

#endif // RECORD_H