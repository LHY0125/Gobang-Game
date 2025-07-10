/**
 * @file network.c
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋网络对战模块实现
 * @version 6.0
 * @date 2025-07-10
 *
 * @copyright Copyright (c) 2025
 */

#include "network.h"
#include "gobang.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
typedef int SOCKET;
#endif

/**
 * @brief 初始化网络模块
 */
bool init_network()
{
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        return false;
    }
#endif
    
    memset(&network_state, 0, sizeof(NetworkGameState));
    network_state.socket = INVALID_SOCKET;
    network_state.port = DEFAULT_PORT;
    
    return true;
}

/**
 * @brief 清理网络模块
 */
void cleanup_network()
{
    if (network_state.socket != INVALID_SOCKET)
    {
        closesocket(network_state.socket);
        network_state.socket = INVALID_SOCKET;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    network_state.is_connected = false;
}

/**
 * @brief 创建服务器（主机模式）
 */
bool create_server(int port)
{
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    
    // 创建套接字
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
    {
        printf("创建套接字失败\n");
        return false;
    }
    
    // 设置地址重用
    int opt = 1;
#ifdef _WIN32
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("绑定端口失败\n");
        closesocket(listen_socket);
        return false;
    }
    
    // 开始监听
    if (listen(listen_socket, 1) == SOCKET_ERROR)
    {
        printf("监听失败\n");
        closesocket(listen_socket);
        return false;
    }
    
    char local_ip[MAX_IP_LENGTH];
    if (get_local_ip(local_ip, sizeof(local_ip)))
    {
        printf("服务器已启动，等待客户端连接...\n");
        printf("本机IP地址: %s\n", local_ip);
        printf("监听端口: %d\n", port);
    } 
    else 
    {
        printf("服务器已启动，监听端口: %d\n", port);
    }
    
    // 等待客户端连接
    SOCKET client_socket = accept(listen_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET)
    {
        printf("接受连接失败\n");
        closesocket(listen_socket);
        return false;
    }
    
    // 关闭监听套接字
    closesocket(listen_socket);
    
    // 保存连接信息
    network_state.socket = client_socket;
    network_state.is_server = true;
    network_state.is_connected = true;
    network_state.local_player_id = PLAYER1;
    network_state.remote_player_id = PLAYER2;
    network_state.port = port;
    strcpy(network_state.remote_ip, inet_ntoa(client_addr.sin_addr));
    
    printf("客户端已连接: %s\n", network_state.remote_ip);
    return true;
}

/**
 * @brief 连接到服务器（客户端模式）
 */
bool connect_to_server(const char* ip, int port)
{
    struct sockaddr_in server_addr;
    
    // 创建套接字
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("创建套接字失败\n");
        return false;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
#ifdef _WIN32
    server_addr.sin_addr.s_addr = inet_addr(ip);
    if (server_addr.sin_addr.s_addr == INADDR_NONE) 
    {
#else
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
#endif
        printf("无效的IP地址: %s\n", ip);
        closesocket(client_socket);
        return false;
    }
    
    printf("正在连接到服务器 %s:%d...\n", ip, port);
    
    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("连接服务器失败\n");
        closesocket(client_socket);
        return false;
    }
    
    // 保存连接信息
    network_state.socket = client_socket;
    network_state.is_server = false;
    network_state.is_connected = true;
    network_state.local_player_id = PLAYER2;
    network_state.remote_player_id = PLAYER1;
    network_state.port = port;
    strcpy(network_state.remote_ip, ip);
    
    printf("成功连接到服务器\n");
    return true;
}

/**
 * @brief 发送网络消息
 */
bool send_network_message(const NetworkMessage* msg)
{
    if (!network_state.is_connected || network_state.socket == INVALID_SOCKET)
    {
        return false;
    }
    
    int bytes_sent = send(network_state.socket, (const char*)msg, sizeof(NetworkMessage), 0);
    return bytes_sent == sizeof(NetworkMessage);
}

/**
 * @brief 接收网络消息
 */
bool receive_network_message(NetworkMessage* msg, int timeout_ms)
{
    if (!network_state.is_connected || network_state.socket == INVALID_SOCKET)
    {
        return false;
    }
    
    // 设置超时
    if (timeout_ms > 0)
    {
#ifdef _WIN32
        DWORD timeout = timeout_ms;
        setsockopt(network_state.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(network_state.socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
    }
    
    int bytes_received = recv(network_state.socket, (char*)msg, sizeof(NetworkMessage), 0);
    
    if (bytes_received == sizeof(NetworkMessage))
    {
        return true;
    } else if (bytes_received == 0)
    {
        // 连接已关闭
        network_state.is_connected = false;
        printf("对方已断开连接\n");
    } else if (bytes_received == SOCKET_ERROR)
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error != WSAETIMEDOUT)
        {
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
#endif
            network_state.is_connected = false;
            printf("网络接收错误\n");
        }
    }
    
    return false;
}

/**
 * @brief 断开网络连接
 */
void disconnect_network()
{
    if (network_state.is_connected)
    {
        NetworkMessage msg = {0};
        msg.type = MSG_DISCONNECT;
        msg.player_id = network_state.local_player_id;
        msg.timestamp = time(NULL);
        
        send_network_message(&msg);
    }
    
    cleanup_network();
}

/**
 * @brief 检查网络连接状态
 */
bool is_network_connected()
{
    return network_state.is_connected && network_state.socket != INVALID_SOCKET;
}

/**
 * @brief 获取本机IP地址
 */
bool get_local_ip(char* ip_buffer, int buffer_size)
{
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        struct hostent* host_entry = gethostbyname(hostname);
        if (host_entry != NULL)
        {
            struct in_addr addr;
            addr.s_addr = *((unsigned long*)host_entry->h_addr_list[0]);
            strncpy(ip_buffer, inet_ntoa(addr), buffer_size - 1);
            ip_buffer[buffer_size - 1] = '\0';
            return true;
        }
    }
#else
    // Linux实现
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != -1)
    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("8.8.8.8");
        addr.sin_port = htons(80);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        {
            socklen_t addr_len = sizeof(addr);
            if (getsockname(sock, (struct sockaddr*)&addr, &addr_len) == 0)
            {
                strncpy(ip_buffer, inet_ntoa(addr.sin_addr), buffer_size - 1);
                ip_buffer[buffer_size - 1] = '\0';
                close(sock);
                return true;
            }
        }
        close(sock);
    }
#endif
    
    // 默认返回本地回环地址
    strncpy(ip_buffer, "127.0.0.1", buffer_size - 1);
    ip_buffer[buffer_size - 1] = '\0';
    return false;
}

/**
 * @brief 发送落子消息
 */
bool send_move(int x, int y, int player_id)
{
    NetworkMessage msg = {0};
    msg.type = MSG_MOVE;
    msg.player_id = player_id;
    msg.x = x;
    msg.y = y;
    msg.timestamp = time(NULL);
    
    return send_network_message(&msg);
}

/**
 * @brief 发送聊天消息
 */
bool send_chat_message(const char* message)
{
    NetworkMessage msg = {0};
    msg.type = MSG_CHAT;
    msg.player_id = network_state.local_player_id;
    strncpy(msg.message, message, sizeof(msg.message) - 1);
    msg.timestamp = time(NULL);
    
    return send_network_message(&msg);
}

/**
 * @brief 发送认输消息
 */
bool send_surrender()
{
    NetworkMessage msg = {0};
    msg.type = MSG_SURRENDER;
    msg.player_id = network_state.local_player_id;
    msg.timestamp = time(NULL);
    
    return send_network_message(&msg);
}

/**
 * @brief 发送悔棋请求
 */
bool send_undo_request(int steps)
{
    NetworkMessage msg = {0};
    msg.type = MSG_UNDO_REQUEST;
    msg.player_id = network_state.local_player_id;
    msg.x = steps; // 使用x字段存储步数
    msg.timestamp = time(NULL);
    
    return send_network_message(&msg);
}

/**
 * @brief 发送悔棋回应
 */
bool send_undo_response(bool accepted, int steps)
{
    NetworkMessage msg = {0};
    msg.type = MSG_UNDO_RESPONSE;
    msg.player_id = network_state.local_player_id;
    msg.x = steps; // 使用x字段存储步数
    msg.y = accepted ? 1 : 0; // 使用y字段存储是否同意
    msg.timestamp = time(NULL);
    
    return send_network_message(&msg);
}