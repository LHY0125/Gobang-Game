/**
 * @file network.h
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋网络对战模块头文件
 * @version 1.0
 * @date 2025-01-15
 *
 * @copyright Copyright (c) 2025
 *
 * @note 本文件定义了五子棋游戏的网络对战功能：
 * 1. 服务器模式（主机）
 * 2. 客户端模式（加入游戏）
 * 3. 网络消息传输
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "gobang.h"
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// 网络配置
#define DEFAULT_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_IP_LENGTH 16

// 消息类型
#define MSG_MOVE 1          // 落子消息
#define MSG_CHAT 2          // 聊天消息
#define MSG_SURRENDER 3     // 认输消息
#define MSG_UNDO_REQUEST 4  // 悔棋请求
#define MSG_UNDO_RESPONSE 5 // 悔棋回应
#define MSG_GAME_START 6    // 游戏开始
#define MSG_GAME_END 7      // 游戏结束
#define MSG_HEARTBEAT 8     // 心跳包
#define MSG_DISCONNECT 9    // 断线消息

// 网络消息结构
typedef struct {
    int type;           // 消息类型
    int player_id;      // 玩家ID
    int x, y;          // 坐标（用于落子）
    char message[256]; // 消息内容（用于聊天等）
    time_t timestamp;  // 时间戳
} NetworkMessage;

// 网络游戏状态
typedef struct {
    SOCKET socket;          // 套接字
    bool is_server;         // 是否为服务器
    bool is_connected;      // 是否已连接
    int local_player_id;    // 本地玩家ID
    int remote_player_id;   // 远程玩家ID
    char remote_ip[MAX_IP_LENGTH]; // 远程IP地址
    int port;              // 端口号
} NetworkGameState;

// 全局变量声明现在在globals.h中

// 函数声明

/**
 * @brief 初始化网络模块
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool init_network();

/**
 * @brief 清理网络模块
 */
void cleanup_network();

/**
 * @brief 创建服务器（主机模式）
 * @param port 监听端口
 * @return true 创建成功
 * @return false 创建失败
 */
bool create_server(int port);

/**
 * @brief 连接到服务器（客户端模式）
 * @param ip 服务器IP地址
 * @param port 服务器端口
 * @return true 连接成功
 * @return false 连接失败
 */
bool connect_to_server(const char* ip, int port);

/**
 * @brief 发送网络消息
 * @param msg 要发送的消息
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_network_message(const NetworkMessage* msg);

/**
 * @brief 接收网络消息
 * @param msg 接收消息的缓冲区
 * @param timeout_ms 超时时间（毫秒），0表示阻塞等待
 * @return true 接收成功
 * @return false 接收失败或超时
 */
bool receive_network_message(NetworkMessage* msg, int timeout_ms);

/**
 * @brief 断开网络连接
 */
void disconnect_network();

/**
 * @brief 检查网络连接状态
 * @return true 连接正常
 * @return false 连接断开
 */
bool is_network_connected();

/**
 * @brief 获取本机IP地址
 * @param ip_buffer 存储IP地址的缓冲区
 * @param buffer_size 缓冲区大小
 * @return true 获取成功
 * @return false 获取失败
 */
bool get_local_ip(char* ip_buffer, int buffer_size);

/**
 * @brief 发送落子消息
 * @param x 行坐标
 * @param y 列坐标
 * @param player_id 玩家ID
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_move(int x, int y, int player_id);

/**
 * @brief 发送聊天消息
 * @param message 聊天内容
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_chat_message(const char* message);

/**
 * @brief 发送认输消息
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_surrender();

/**
 * @brief 发送悔棋请求
 * @param steps 悔棋步数
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_undo_request(int steps);

/**
 * @brief 发送悔棋回应
 * @param accepted 是否同意悔棋
 * @param steps 悔棋步数
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_undo_response(bool accepted, int steps);

#endif // NETWORK_H