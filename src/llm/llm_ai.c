/**
 * @file llm_ai.c
 * @brief 大模型AI模块实现
 * @note 通过OpenAI兼容API调用大模型进行五子棋对弈
 *       支持 MiniMax、DeepSeek、GPT 等兼容接口
 */

#include "llm_ai.h"
#include "globals.h"
#include "config.h"
#include "gobang.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <process.h>
#endif

// ==================== 内部函数声明 ====================

static bool http_post_json(const char *url, const char *api_key,
                           const char *json_body, char *response, int response_size);
static char *build_prompt(void);
static char *build_request_json(const char *prompt);
static bool parse_response(const char *response, int *out_x, int *out_y);
static bool extract_coords(const char *text, int *out_x, int *out_y);

// ==================== 异步请求支持 ====================

#ifdef _WIN32

typedef struct {
    int result;    // 0=思考中, 1=成功, -1=失败
    int x, y;      // 成功时的坐标
    HANDLE thread; // 后台线程句柄
} LLMAsyncResult;

static LLMAsyncResult g_llm_async = {0, 0, 0, NULL};

static unsigned __stdcall llm_thread_func(void *arg)
{
    (void)arg;
    int x = -1, y = -1;
    bool ok = llm_ai_move(&x, &y);
    g_llm_async.x = x;
    g_llm_async.y = y;
    g_llm_async.result = ok ? 1 : -1;
    return 0;
}

void llm_ai_start_move(void)
{
    // 等待上一次线程结束（如果有）
    if (g_llm_async.thread)
    {
        WaitForSingleObject(g_llm_async.thread, INFINITE);
        CloseHandle(g_llm_async.thread);
        g_llm_async.thread = NULL;
    }

    g_llm_async.result = 0;
    g_llm_async.x = -1;
    g_llm_async.y = -1;

    g_llm_async.thread = (HANDLE)_beginthreadex(NULL, 0, llm_thread_func, NULL, 0, NULL);
}

int llm_ai_poll_result(int *out_x, int *out_y)
{
    if (g_llm_async.thread == NULL)
        return -1;

    // 检查线程是否完成
    DWORD wait = WaitForSingleObject(g_llm_async.thread, 0);
    if (wait == WAIT_OBJECT_0)
    {
        // 线程已完成
        CloseHandle(g_llm_async.thread);
        g_llm_async.thread = NULL;
        *out_x = g_llm_async.x;
        *out_y = g_llm_async.y;
        return g_llm_async.result;
    }

    return 0; // 仍在思考
}

#else
// 非Windows平台的同步回退
void llm_ai_start_move(void) {}

int llm_ai_poll_result(int *out_x, int *out_y)
{
    (void)out_x;
    (void)out_y;
    return -1;
}
#endif

// ==================== 公共接口 ====================

bool llm_ai_move(int *out_x, int *out_y)
{
    if (llm_api_key[0] == '\0')
    {
        printf("[LLM] 错误：未配置API Key\n");
        return false;
    }

    for (int retry = 0; retry < LLM_MAX_RETRIES; retry++)
    {
        // 1. 构造 prompt
        char *prompt = build_prompt();
        if (!prompt)
            return false;

        // 2. 构造 JSON 请求体
        char *json_body = build_request_json(prompt);
        free(prompt);
        if (!json_body)
            return false;

        // 3. 发送 HTTP 请求
        char response[8192] = {0};
        bool ok = http_post_json(llm_endpoint, llm_api_key, json_body, response, sizeof(response));
        free(json_body);

        if (!ok)
        {
            printf("[LLM] HTTP请求失败 (第%d次)\n", retry + 1);
            continue;
        }

        // 4. 解析响应
        if (parse_response(response, out_x, out_y))
        {
            // 5. 验证坐标合法性
            if (*out_x >= 0 && *out_x < BOARD_SIZE &&
                *out_y >= 0 && *out_y < BOARD_SIZE &&
                board[*out_x][*out_y] == EMPTY)
            {
                printf("[LLM] 落子(%d, %d)\n", *out_x, *out_y);
                return true;
            }
            printf("[LLM] 坐标(%d, %d)非法，重试 (%d/%d)\n", *out_x, *out_y, retry + 1, LLM_MAX_RETRIES);
        }
        else
        {
            printf("[LLM] 解析响应失败，重试 (%d/%d)\n", retry + 1, LLM_MAX_RETRIES);
        }
    }

    return false;
}

// ==================== Prompt 构造 ====================

// 检查坐标是否在棋盘范围内
static bool in_board(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

static char *build_prompt(void)
{
    // 估算所需空间
    int max_len = 3072 + step_count * 20 + 512;
    char *buf = (char *)malloc(max_len);
    if (!buf)
        return NULL;

    int pos = 0;

    // 棋盘基本信息（不用()格式，避免被坐标提取误匹配）
    pos += snprintf(buf + pos, max_len - pos,
        "棋盘 %d×%d，坐标范围 0-%d\n"
        "你=白O，对手=黑X\n\n",
        BOARD_SIZE, BOARD_SIZE, BOARD_SIZE - 1);

    // 黑子位置（用方括号格式，不用圆括号）
    pos += snprintf(buf + pos, max_len - pos, "黑子X位置:");
    int black_count = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == PLAYER || board[i][j] == PLAYER1)
            {
                pos += snprintf(buf + pos, max_len - pos, " [%d,%d]", i, j);
                black_count++;
            }
    if (black_count == 0)
        pos += snprintf(buf + pos, max_len - pos, " 无");
    pos += snprintf(buf + pos, max_len - pos, "\n");

    // 白子位置
    pos += snprintf(buf + pos, max_len - pos, "白子O位置:");
    int white_count = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == AI || board[i][j] == PLAYER2)
            {
                pos += snprintf(buf + pos, max_len - pos, " [%d,%d]", i, j);
                white_count++;
            }
    if (white_count == 0)
        pos += snprintf(buf + pos, max_len - pos, " 无");
    pos += snprintf(buf + pos, max_len - pos, "\n");

    // 最近走法
    if (step_count > 0)
    {
        int show_count = step_count < 6 ? step_count : 6;
        pos += snprintf(buf + pos, max_len - pos, "\n最近%d步:\n", show_count);
        for (int i = step_count - show_count; i < step_count; i++)
        {
            const char *who = (steps[i].player == PLAYER || steps[i].player == PLAYER1) ? "X" : "O";
            pos += snprintf(buf + pos, max_len - pos, "  %s [%d,%d]\n", who, steps[i].x, steps[i].y);
        }
    }

    // 收集候选空位（已有棋子周围2格内的空位）
    char candidate[BOARD_SIZE][BOARD_SIZE];
    memset(candidate, 0, sizeof(candidate));

    int total_stones = black_count + white_count;
    if (total_stones == 0)
    {
        candidate[BOARD_SIZE / 2][BOARD_SIZE / 2] = 1;
    }
    else
    {
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                if (board[i][j] != EMPTY)
                {
                    // 标记周围2格内的空位
                    for (int di = -2; di <= 2; di++)
                    {
                        for (int dj = -2; dj <= 2; dj++)
                        {
                            int ni = i + di, nj = j + dj;
                            if (in_board(ni, nj) && board[ni][nj] == EMPTY)
                                candidate[ni][nj] = 1;
                        }
                    }
                }
            }
        }
    }

    // 输出候选位置（用[]格式，避免与LLM回复的()格式冲突）
    int cand_count = 0;
    pos += snprintf(buf + pos, max_len - pos, "\n可选空位:\n");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (candidate[i][j])
            {
                pos += snprintf(buf + pos, max_len - pos, "[%d,%d] ", i, j);
                cand_count++;
                if (cand_count % 10 == 0)
                    pos += snprintf(buf + pos, max_len - pos, "\n");
            }
        }
    }
    pos += snprintf(buf + pos, max_len - pos, "\n共%d个可选。\n", cand_count);

    // 输出格式要求
    pos += snprintf(buf + pos, max_len - pos,
        "\n从上面选一个最佳位置，用(行,列)格式回复。只回复坐标。\n");

    return buf;
}

// ==================== JSON 构造 ====================

static char *build_request_json(const char *prompt)
{
    cJSON *root = cJSON_CreateObject();
    if (!root)
        return NULL;

    cJSON_AddStringToObject(root, "model", llm_model);

    // messages 数组
    cJSON *messages = cJSON_AddArrayToObject(root, "messages");

    // system 消息
    cJSON *sys_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(sys_msg, "role", "system");
    cJSON_AddStringToObject(sys_msg, "content",
        "你是五子棋AI。从给定的可选空位列表中选一个最佳位置落子。"
        "只回复(行,列)格式的坐标，不要任何解释。");
    cJSON_AddItemToArray(messages, sys_msg);

    // user 消息（包含棋盘状态）
    cJSON *user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(user_msg, "role", "user");
    cJSON_AddStringToObject(user_msg, "content", prompt);
    cJSON_AddItemToArray(messages, user_msg);

    // 其他参数
    // 推理模型需要更多token（思考+输出），非推理模型够用即可
    cJSON_AddNumberToObject(root, "temperature", 0.1);
    cJSON_AddNumberToObject(root, "max_tokens", 512);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

// ==================== HTTP 请求（WinHTTP）====================

#ifdef _WIN32

static bool http_post_json(const char *url, const char *api_key,
                           const char *json_body, char *response, int response_size)
{
    bool success = false;
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

    // 解析 URL：提取 host、path、端口、是否 HTTPS
    WCHAR whost[256] = {0};
    WCHAR wpath[512] = {0};
    wchar_t wauth[256] = {0};
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;
    BOOL is_https = TRUE;

    // 简单解析 URL
    const char *host_start = url;
    const char *path_start = "/";
    int host_len = 0;

    if (strncmp(url, "https://", 8) == 0)
    {
        host_start = url + 8;
        is_https = TRUE;
        port = INTERNET_DEFAULT_HTTPS_PORT;
    }
    else if (strncmp(url, "http://", 7) == 0)
    {
        host_start = url + 7;
        is_https = FALSE;
        port = INTERNET_DEFAULT_HTTP_PORT;
    }

    // 找到 path 的起始位置
    const char *p = strchr(host_start, '/');
    if (p)
    {
        host_len = (int)(p - host_start);
        path_start = p;
    }
    else
    {
        host_len = (int)strlen(host_start);
        path_start = "/";
    }

    // 检查是否有端口号
    const char *colon = strchr(host_start, ':');
    if (colon && colon < host_start + host_len)
    {
        host_len = (int)(colon - host_start);
        port = (INTERNET_PORT)atoi(colon + 1);
    }

    // 转换为宽字符
    MultiByteToWideChar(CP_UTF8, 0, host_start, host_len, whost, 256);
    MultiByteToWideChar(CP_UTF8, 0, path_start, -1, wpath, 512);

    // 构造 Authorization header
    wchar_t wapi_key[128] = {0};
    MultiByteToWideChar(CP_UTF8, 0, api_key, -1, wapi_key, 128);
    swprintf(wauth, 256, L"Authorization: Bearer %s", wapi_key);

    // 打开 WinHTTP 会话
    hSession = WinHttpOpen(L"Gobang/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        goto cleanup;

    // 设置超时
    WinHttpSetTimeouts(hSession, 5000, 10000, 15000, LLM_TIMEOUT_MS);

    // 连接服务器
    hConnect = WinHttpConnect(hSession, whost, port, 0);
    if (!hConnect)
        goto cleanup;

    // 创建请求
    hRequest = WinHttpOpenRequest(hConnect, L"POST", wpath,
                                  NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  is_https ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest)
        goto cleanup;

    // 添加请求头
    WinHttpAddRequestHeaders(hRequest, wauth, -1L, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json", -1L, WINHTTP_ADDREQ_FLAG_ADD);

    // 发送请求
    int body_len = (int)strlen(json_body);
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            (LPVOID)json_body, body_len, body_len, 0))
        goto cleanup;

    // 接收响应
    if (!WinHttpReceiveResponse(hRequest, NULL))
        goto cleanup;

    // 读取响应体
    {
        int total_read = 0;
        DWORD bytes_available = 0;
        DWORD bytes_read = 0;

        while (WinHttpQueryDataAvailable(hRequest, &bytes_available) && bytes_available > 0)
        {
            if (total_read + (int)bytes_available >= response_size - 1)
                break;

            WinHttpReadData(hRequest, response + total_read, bytes_available, &bytes_read);
            total_read += bytes_read;
            bytes_available = 0;
        }
        response[total_read] = '\0';
        success = (total_read > 0);
    }

cleanup:
    if (hRequest)
        WinHttpCloseHandle(hRequest);
    if (hConnect)
        WinHttpCloseHandle(hConnect);
    if (hSession)
        WinHttpCloseHandle(hSession);
    return success;
}

#else
// 非 Windows 平台的空实现
static bool http_post_json(const char *url, const char *api_key,
                           const char *json_body, char *response, int response_size)
{
    (void)url;
    (void)api_key;
    (void)json_body;
    (void)response;
    (void)response_size;
    return false;
}
#endif

// ==================== 响应解析 ====================

static bool parse_response(const char *response, int *out_x, int *out_y)
{
    cJSON *root = cJSON_Parse(response);
    if (!root)
    {
        printf("[LLM] JSON解析失败\n");
        return false;
    }

    // OpenAI 格式：choices[0].message.content
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (!choices || !cJSON_IsArray(choices))
    {
        // 兼容某些API的错误格式
        cJSON *error = cJSON_GetObjectItem(root, "error");
        if (error)
        {
            cJSON *msg = cJSON_GetObjectItem(error, "message");
            if (msg && cJSON_IsString(msg))
                printf("[LLM] API错误: %s\n", msg->valuestring);
        }
        cJSON_Delete(root);
        return false;
    }

    cJSON *first = cJSON_GetArrayItem(choices, 0);
    if (!first)
    {
        cJSON_Delete(root);
        return false;
    }

    // 提取 content
    cJSON *message = cJSON_GetObjectItem(first, "message");
    cJSON *content = NULL;
    if (message)
        content = cJSON_GetObjectItem(message, "content");
    else
        content = cJSON_GetObjectItem(first, "text"); // 某些API直接返回text

    if (!content || !cJSON_IsString(content))
    {
        cJSON_Delete(root);
        return false;
    }

    printf("[LLM] 模型回复: %s\n", content->valuestring);
    bool ok = extract_coords(content->valuestring, out_x, out_y);
    cJSON_Delete(root);
    return ok;
}

// ==================== 坐标提取 ====================

// 跳过所有 <think>...</think> 块，返回处理后的文本（调用者需 free）
static char *strip_think_tags(const char *text)
{
    int len = (int)strlen(text);
    char *buf = (char *)malloc(len + 1);
    if (!buf)
        return NULL;

    int out = 0;
    const char *p = text;

    while (*p)
    {
        const char *think_start = strstr(p, "<think>");
        if (think_start == p)
        {
            // 找到 <think> 标签，跳到 </think> 之后
            const char *think_end = strstr(p, "</think>");
            if (think_end)
            {
                p = think_end + 8; // strlen("</think>")
                continue;
            }
            else
            {
                // 没有闭合的 <think>，跳过剩余内容
                break;
            }
        }

        // 复制 <think> 之前的普通文本
        int copy_len = think_start ? (int)(think_start - p) : (int)strlen(p);
        memcpy(buf + out, p, copy_len);
        out += copy_len;
        p += copy_len;
    }

    buf[out] = '\0';
    return buf;
}

// 在文本中查找最后一个 (行,列) 坐标
static bool find_last_coord(const char *text, int *out_x, int *out_y)
{
    int last_x = -1, last_y = -1;
    bool found = false;
    const char *p = text;

    while (*p)
    {
        if (*p == '(')
        {
            int x = -1, y = -1;
            if (sscanf(p, "(%d,%d)", &x, &y) == 2 ||
                sscanf(p, "(%d, %d)", &x, &y) == 2)
            {
                if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE)
                {
                    last_x = x;
                    last_y = y;
                    found = true;
                }
            }
        }
        p++;
    }

    if (found)
    {
        *out_x = last_x;
        *out_y = last_y;
    }
    return found;
}

static bool extract_coords(const char *text, int *out_x, int *out_y)
{
    // 第一步：去掉 <think>...</think>，从回复正文中提取
    char *clean = strip_think_tags(text);
    if (clean)
    {
        // 跳过空白字符
        const char *p = clean;
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
            p++;

        if (*p && find_last_coord(p, out_x, out_y))
        {
            free(clean);
            return true;
        }
        free(clean);
    }

    // 第二步（兜底）：从完整文本（含推理）中提取最后一个坐标
    // 推理模型可能把最终答案写在 <think> 标签里
    return find_last_coord(text, out_x, out_y);
}
