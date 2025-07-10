/**
 * @file network.h
 * @author ������(3364451258@qq.com��15236416560@163.com��lhy3364451258@outlook.com)
 * @brief �����������սģ��ͷ�ļ�
 * @version 1.0
 * @date 2025-01-15
 *
 * @copyright Copyright (c) 2025
 *
 * @note ���ļ���������������Ϸ�������ս���ܣ�
 * 1. ������ģʽ��������
 * 2. �ͻ���ģʽ��������Ϸ��
 * 3. ������Ϣ����
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

// ��������
#define DEFAULT_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_IP_LENGTH 16

// ��Ϣ����
#define MSG_MOVE 1          // ������Ϣ
#define MSG_CHAT 2          // ������Ϣ
#define MSG_SURRENDER 3     // ������Ϣ
#define MSG_UNDO_REQUEST 4  // ��������
#define MSG_UNDO_RESPONSE 5 // �����Ӧ
#define MSG_GAME_START 6    // ��Ϸ��ʼ
#define MSG_GAME_END 7      // ��Ϸ����
#define MSG_HEARTBEAT 8     // ������
#define MSG_DISCONNECT 9    // ������Ϣ

// ������Ϣ�ṹ
typedef struct {
    int type;           // ��Ϣ����
    int player_id;      // ���ID
    int x, y;          // ���꣨�������ӣ�
    char message[256]; // ��Ϣ���ݣ���������ȣ�
    time_t timestamp;  // ʱ���
} NetworkMessage;

// ������Ϸ״̬
typedef struct {
    SOCKET socket;          // �׽���
    bool is_server;         // �Ƿ�Ϊ������
    bool is_connected;      // �Ƿ�������
    int local_player_id;    // �������ID
    int remote_player_id;   // Զ�����ID
    char remote_ip[MAX_IP_LENGTH]; // Զ��IP��ַ
    int port;              // �˿ں�
} NetworkGameState;

// ȫ�ֱ�������������globals.h��

// ��������

/**
 * @brief ��ʼ������ģ��
 * @return true ��ʼ���ɹ�
 * @return false ��ʼ��ʧ��
 */
bool init_network();

/**
 * @brief ��������ģ��
 */
void cleanup_network();

/**
 * @brief ����������������ģʽ��
 * @param port �����˿�
 * @return true �����ɹ�
 * @return false ����ʧ��
 */
bool create_server(int port);

/**
 * @brief ���ӵ����������ͻ���ģʽ��
 * @param ip ������IP��ַ
 * @param port �������˿�
 * @return true ���ӳɹ�
 * @return false ����ʧ��
 */
bool connect_to_server(const char* ip, int port);

/**
 * @brief ����������Ϣ
 * @param msg Ҫ���͵���Ϣ
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_network_message(const NetworkMessage* msg);

/**
 * @brief ����������Ϣ
 * @param msg ������Ϣ�Ļ�����
 * @param timeout_ms ��ʱʱ�䣨���룩��0��ʾ�����ȴ�
 * @return true ���ճɹ�
 * @return false ����ʧ�ܻ�ʱ
 */
bool receive_network_message(NetworkMessage* msg, int timeout_ms);

/**
 * @brief �Ͽ���������
 */
void disconnect_network();

/**
 * @brief �����������״̬
 * @return true ��������
 * @return false ���ӶϿ�
 */
bool is_network_connected();

/**
 * @brief ��ȡ����IP��ַ
 * @param ip_buffer �洢IP��ַ�Ļ�����
 * @param buffer_size ��������С
 * @return true ��ȡ�ɹ�
 * @return false ��ȡʧ��
 */
bool get_local_ip(char* ip_buffer, int buffer_size);

/**
 * @brief ����������Ϣ
 * @param x ������
 * @param y ������
 * @param player_id ���ID
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_move(int x, int y, int player_id);

/**
 * @brief ����������Ϣ
 * @param message ��������
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_chat_message(const char* message);

/**
 * @brief ����������Ϣ
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_surrender();

/**
 * @brief ���ͻ�������
 * @param steps ���岽��
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_undo_request(int steps);

/**
 * @brief ���ͻ����Ӧ
 * @param accepted �Ƿ�ͬ�����
 * @param steps ���岽��
 * @return true ���ͳɹ�
 * @return false ����ʧ��
 */
bool send_undo_response(bool accepted, int steps);

#endif // NETWORK_H