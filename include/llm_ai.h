/**
 * @file llm_ai.h
 * @brief 大模型AI模块头文件
 * @note 通过OpenAI兼容API调用大模型进行五子棋对弈
 */

#ifndef LLM_AI_H
#define LLM_AI_H

#include <stdbool.h>

/**
 * @brief 调用大模型获取落子坐标（同步，会阻塞）
 * @param out_x 输出：落子行坐标 (0-based)
 * @param out_y 输出：落子列坐标 (0-based)
 * @return true 获取成功，坐标合法且位置为空
 * @return false 获取失败（网络错误/坐标非法/重试耗尽）
 */
bool llm_ai_move(int *out_x, int *out_y);

/**
 * @brief 异步启动大模型思考（后台线程）
 * @note 调用后用 llm_ai_poll_result 轮询结果
 */
void llm_ai_start_move(void);

/**
 * @brief 轮询大模型结果（非阻塞）
 * @param out_x 输出：落子行坐标
 * @param out_y 输出：落子列坐标
 * @return 0 仍在思考, 1 成功获取坐标, -1 失败（应回退算法AI）
 */
int llm_ai_poll_result(int *out_x, int *out_y);

#endif // LLM_AI_H
