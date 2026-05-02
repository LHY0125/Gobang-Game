/**
 * @file network.c
 * @author 刘航宇(3364451258@qq.com、15236416560@163.com、lhy3364451258@outlook.com)
 * @brief 五子棋网络对战模块实现
 */

#include "network.h"
#include "gobang.h"
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <enet/enet.h>

/**
 * @brief 初始化网络模块
 */
static bool enet_initialized = false;

bool init_network()
{
    if (!enet_initialized)
    {
        if (enet_initialize() != 0)
        {
            printf("An error occurred while initializing ENet.\n");
            return false;
        }
        enet_initialized = true;
    }

    memset(&network_state, 0, sizeof(NetworkGameState));
    network_state.port = DEFAULT_NETWORK_PORT;

    return true;
}

/**
 * @brief 清理网络模块
 */
void cleanup_network()
{
    disconnect_network();

    if (network_state.host != NULL)
    {
        enet_host_destroy((ENetHost *)network_state.host);
        network_state.host = NULL;
    }

    if (enet_initialized)
    {
        enet_deinitialize();
        enet_initialized = false;
    }

    memset(&network_state, 0, sizeof(NetworkGameState));
}

/**
 * @brief 创建服务器（主机模式）
 */
bool create_server(int port)
{
    if (!init_network())
        return false;

    ENetAddress address;

    // 绑定所有接口
    address.host = ENET_HOST_ANY;
    address.port = port;

    // 创建服务器主机
    network_state.host = (void *)enet_host_create(&address,
                                                  1, // 仅允许1个客户端连接
                                                  2, // 允许2个通道 (0 和 1)
                                                  0, // 假设传入带宽无限制
                                                  0  // 假设传出带宽无限制
    );

    if (network_state.host == NULL)
    {
        printf("创建服务器失败\n");
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

    // 阻塞等待客户端连接
    ENetEvent event;
    printf("等待连接...\n");
    // 等待长一点的时间，比如60秒，或者在真实应用中应该放在循环里非阻塞检查
    if (enet_host_service((ENetHost *)network_state.host, &event, 60000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        network_state.peer = (void *)event.peer;
        network_state.is_server = true;
        network_state.is_connected = true;
        network_state.local_player_id = PLAYER1;
        network_state.remote_player_id = PLAYER2;
        network_state.port = port;

        enet_address_get_host_ip(&event.peer->address, network_state.remote_ip, sizeof(network_state.remote_ip));

        printf("客户端已连接: %s\n", network_state.remote_ip);
        return true;
    }

    // 超时或失败
    printf("等待连接超时或失败\n");
    enet_host_destroy((ENetHost *)network_state.host);
    network_state.host = NULL;
    return false;
}

/**
 * @brief 连接到服务器（客户端模式）
 */
bool connect_to_server(const char *ip, int port)
{
    if (!init_network())
        return false;

    // 创建客户端主机
    network_state.host = (void *)enet_host_create(NULL, // 创建客户端
                                                  1,    // 仅允许1个传出连接
                                                  2,    // 允许2个通道 (0 和 1)
                                                  0,    // 假设传入带宽无限制
                                                  0     // 假设传出带宽无限制
    );

    if (network_state.host == NULL)
    {
        printf("创建客户端主机失败\n");
        return false;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    enet_address_set_host(&address, ip);
    address.port = port;

    printf("正在连接到服务器 %s:%d...\n", ip, port);

    peer = enet_host_connect((ENetHost *)network_state.host, &address, 2, 0);
    if (peer == NULL)
    {
        printf("没有可用的对等端来启动连接\n");
        enet_host_destroy((ENetHost *)network_state.host);
        network_state.host = NULL;
        return false;
    }

    // 等待连接成功
    if (enet_host_service((ENetHost *)network_state.host, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        network_state.peer = (void *)peer;
        network_state.is_server = false;
        network_state.is_connected = true;
        network_state.local_player_id = PLAYER2;
        network_state.remote_player_id = PLAYER1;
        network_state.port = port;
        strncpy(network_state.remote_ip, ip, sizeof(network_state.remote_ip) - 1);

        printf("成功连接到服务器\n");
        return true;
    }

    // 连接失败
    printf("连接服务器失败\n");
    enet_peer_reset(peer);
    enet_host_destroy((ENetHost *)network_state.host);
    network_state.host = NULL;
    return false;
}

/**
 * @brief 发送网络消息
 */
bool send_network_message(const NetworkMessage *msg)
{
    if (!network_state.is_connected || network_state.peer == NULL)
    {
        return false;
    }

    ENetPacket *packet = enet_packet_create(msg, sizeof(NetworkMessage), ENET_PACKET_FLAG_RELIABLE);
    if (enet_peer_send((ENetPeer *)network_state.peer, 0, packet) < 0)
    {
        enet_packet_destroy(packet); // 发送失败需手动销毁
        return false;
    }

    // 强制发送
    enet_host_flush((ENetHost *)network_state.host);
    return true;
}

/**
 * @brief 接收网络消息
 */
bool receive_network_message(NetworkMessage *msg, int timeout_ms)
{
    if (!network_state.is_connected || network_state.host == NULL)
    {
        return false;
    }

    ENetEvent event;
    int serviceResult = enet_host_service((ENetHost *)network_state.host, &event, timeout_ms);

    if (serviceResult > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            if (event.packet->dataLength == sizeof(NetworkMessage))
            {
                memcpy(msg, event.packet->data, sizeof(NetworkMessage));
                enet_packet_destroy(event.packet);
                return true;
            }
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            network_state.is_connected = false;
            printf("对方已断开连接\n");
            network_state.peer = NULL;
            break;

        case ENET_EVENT_TYPE_NONE:
        case ENET_EVENT_TYPE_CONNECT:
            break;
        }
    }
    else if (serviceResult < 0)
    {
        network_state.is_connected = false;
        printf("网络接收错误\n");
    }

    return false;
}

/**
 * @brief 断开网络连接
 */
void disconnect_network()
{
    if (network_state.is_connected && network_state.peer != NULL)
    {
        ENetEvent event;

        enet_peer_disconnect((ENetPeer *)network_state.peer, 0);

        // 等待断开确认
        while (enet_host_service((ENetHost *)network_state.host, &event, 3000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                puts("断开连接成功");
                goto DONE;
            default:
                break;
            }
        }

        // 超时强制重置
        enet_peer_reset((ENetPeer *)network_state.peer);
    DONE:
        network_state.is_connected = false;
        network_state.peer = NULL;
    }
}

/**
 * @brief 检查网络连接状态
 */
bool is_network_connected()
{
    return network_state.is_connected && network_state.peer != NULL;
}

/**
 * @brief 获取本机局域网IP地址
 */
bool get_local_ip(char *ip_buffer, int buffer_size)
{
#ifdef _WIN32
    // 使用 Winsock 获取本机 IP（ws2_32 已链接）
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
    {
        strncpy(ip_buffer, "127.0.0.1", buffer_size - 1);
        ip_buffer[buffer_size - 1] = '\0';
        return false;
    }

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL || host->h_addr_list[0] == NULL)
    {
        strncpy(ip_buffer, "127.0.0.1", buffer_size - 1);
        ip_buffer[buffer_size - 1] = '\0';
        return false;
    }

    // 遍历所有地址，找一个非回环的 IPv4 地址
    for (int i = 0; host->h_addr_list[i] != NULL; i++)
    {
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
        const char *ip = inet_ntoa(addr);
        if (ip && strcmp(ip, "127.0.0.1") != 0)
        {
            strncpy(ip_buffer, ip, buffer_size - 1);
            ip_buffer[buffer_size - 1] = '\0';
            return true;
        }
    }

    // 没找到非回环地址，返回第一个
    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
    const char *ip = inet_ntoa(addr);
    strncpy(ip_buffer, ip ? ip : "127.0.0.1", buffer_size - 1);
    ip_buffer[buffer_size - 1] = '\0';
    return true;
#else
    strncpy(ip_buffer, "127.0.0.1", buffer_size - 1);
    ip_buffer[buffer_size - 1] = '\0';
    return true;
#endif
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
bool send_chat_message(const char *message)
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
    msg.x = steps;            // 使用x字段存储步数
    msg.y = accepted ? 1 : 0; // 使用y字段存储是否同意
    msg.timestamp = time(NULL);

    return send_network_message(&msg);
}