/**
 * @file type.h
 * @brief 五子棋游戏数据类型定义头文件
 * @note 本文件集中定义了五子棋游戏中使用的所有数据结构和枚举类型
 * @author 刘航宇
 */

#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <time.h>
#include <stdint.h>

// ==================== 游戏核心数据结构 ====================

/**
 * @brief 记录一步棋的详细信息
 */
typedef struct
{
    int player; // 执行该步的玩家标识
    int x;      // 落子的行坐标 (0-based)
    int y;      // 落子的列坐标 (0-based)
} Step;

/**
 * @brief 存储在特定方向上棋子连续性的信息
 * @details 用于评估棋形，例如判断活三、冲四等关键形态
 */
typedef struct
{
    int continuous_chess; // 连续同色棋子的数量
    bool check_start;     // 棋子序列的起始端是否为空位（即是否开放）
    bool check_end;       // 棋子序列的末尾端是否为空位（即是否开放）
} DirInfo;

// ==================== AI相关数据结构 ====================

/**
 * @brief 移动排序结构体
 * @details 用于AI移动排序，存储候选移动及其评估分数
 */
typedef struct
{
    int x, y;  // 位置坐标
    int score; // 评估分数
} ScoredMove;

/**
 * @brief 威胁类型枚举
 * @details 用于AI威胁检测系统
 */
typedef enum
{
    THREAT_NONE = 0,     // 无威胁
    THREAT_WIN = 5,      // 直接获胜
    THREAT_FOUR = 4,     // 活四/冲四
    THREAT_THREE = 3,    // 活三
    THREAT_DOUBLE = 2,   // 双威胁
    THREAT_POTENTIAL = 1 // 潜在威胁
} ThreatLevel;

// ==================== 网络相关数据结构 ====================

/**
 * @brief 网络消息结构
 * @details 用于网络对战中的消息传输
 */
typedef struct
{
    int type;          // 消息类型
    int player_id;     // 玩家ID
    int x, y;          // 坐标（用于落子）
    char message[256]; // 消息内容（用于聊天等）
    time_t timestamp;  // 时间戳
} NetworkMessage;

/**
 * @brief 网络游戏状态结构
 * @details 用于管理网络游戏状态
 */
// ENet 头文件需要前置声明或直接包含，但在 type.h 中包含可能引起循环依赖
// 我们可以使用 void* 来避免在 type.h 中包含 enet.h
typedef struct
{
    void *host;          // ENetHost * (Server 或 Client)
    void *peer;          // ENetPeer * (连接的对象)
    bool is_server;      // 是否为服务器(主机)
    bool is_connected;   // 是否已连接
    int local_player_id; // 本地玩家ID
    int remote_player_id; // 远程玩家ID
    char remote_ip[64];  // 远程IP
    int port;            // 端口
} NetworkGameState;

#endif // TYPE_H