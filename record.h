#ifndef RECORD_H
#define RECORD_H

#include "gobang.h"

// --- 复盘与记录 ---
/**
 * @brief 进入复盘流程，回顾整局游戏
 * @param game_mode 游戏模式（1为人机，2为双人）
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
 * @return 0表示成功，非0表示失败
 */
int load_game_from_file(const char *filename);

#endif // RECORD_H