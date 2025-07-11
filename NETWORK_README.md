# 五子棋网络对战功能说明

## 功能概述

本项目新增了网络对战功能，允许两台设备通过网络进行在线五子棋对战。支持局域网和互联网连接。

## 编译方法

```bash
gcc -o gobang.exe main.c gobang.c game_mode.c ai.c record.c init_board.c ui.c config.c network.c -lws2_32
```

**注意：** 
- Windows系统需要链接 `-lws2_32` 网络库
- Linux系统不需要额外的网络库链接

## 使用方法

### 1. 启动游戏
运行编译后的程序：
```bash
.\gobang.exe
```

### 2. 选择网络对战模式
在主菜单中选择 `3. 网络对战`

### 3. 选择连接模式

#### 模式1：创建房间（作为主机）
- 选择 `1. 创建房间（作为主机）`
- 输入监听端口（默认8888，建议使用1024-65535范围内的端口）
- 程序会显示本机IP地址，将此IP告知对方玩家
- 等待对方连接

#### 模式2：加入房间（连接到主机）
- 选择 `2. 加入房间（连接到主机）`
- 输入主机的IP地址
- 输入主机的端口号（与主机设置的端口一致）
- 连接到主机

### 4. 开始游戏
- 连接成功后，游戏自动开始
- 主机为玩家1（●），客户端为玩家2（○）
- 玩家1先手

## 游戏操作

### 基本操作
- **落子**：输入坐标 `行号 列号`（如：`7 7`）
- **认输**：输入 `S` 或 `s`
- **悔棋**：输入 `R` 或 `r`（需要对方同意）

### 网络功能
- **自动同步**：落子操作会自动同步到对方
- **断线检测**：自动检测网络连接状态
- **超时处理**：支持回合时间限制（如果启用计时器）
- **悔棋协商**：悔棋需要对方同意才能生效

## 网络配置

### 端口设置
- 默认端口：8888
- 可用端口范围：1024-65535
- 确保防火墙允许所选端口的通信

### IP地址
- **局域网**：使用内网IP地址（如：192.168.1.100）
- **互联网**：使用公网IP地址，可能需要路由器端口转发

### 防火墙设置
如果连接失败，请检查防火墙设置：

#### Windows防火墙
1. 打开Windows安全中心
2. 选择「防火墙和网络保护」
3. 选择「允许应用通过防火墙」
4. 添加gobang.exe到允许列表

#### 路由器设置（互联网对战）
如需通过互联网对战，主机方需要：
1. 在路由器中设置端口转发
2. 将选定端口转发到主机的内网IP
3. 将路由器的公网IP告知对方

## 故障排除

### 常见问题

1. **连接失败**
   - 检查IP地址和端口是否正确
   - 确认防火墙设置
   - 确保两台设备网络连通

2. **游戏中断**
   - 检查网络连接稳定性
   - 重新启动游戏并重新连接

3. **端口被占用**
   - 更换其他端口号
   - 关闭占用端口的其他程序

### 网络测试
可以使用以下命令测试网络连通性：
```bash
# 测试连通性
ping <对方IP地址>

# 测试端口（需要telnet客户端）
telnet <对方IP地址> <端口号>
```

## 技术特性

- **协议**：TCP/IP
- **消息格式**：自定义二进制协议
- **支持功能**：
  - 落子同步
  - 认输处理
  - 悔棋协商
  - 断线检测
  - 心跳保活

## 安全注意事项

1. **局域网使用**：相对安全，适合家庭或办公室环境
2. **互联网使用**：
   - 不要使用默认端口8888
   - 游戏结束后及时关闭程序
   - 注意保护个人网络信息

## 更新日志

### v1.0 (2025-01-15)
- 新增网络对战功能
- 支持TCP/IP连接
- 实现落子同步
- 添加悔棋协商机制
- 支持断线检测
- 兼容现有游戏功能（计时器、禁手规则等）

---

**开发者：** 刘航宇  
**联系邮箱：** 3364451258@qq.com  
**项目主页：** https://github.com/LHY0125/Gobang-Game