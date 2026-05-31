# Gobang 网络对战设计文档

> 状态: 已确认 | 日期: 2026-05-31

## 目标

为 Gobang v2.0 实现基于 renet 的 P2P 网络对战，两人通过 IP:端口直连对弈。

## 架构

```
┌─ React 前端 ───────────────────────────────────────────┐
│  OnlineSetup                  GameView                  │
│  创建房间/加入房间             连接状态指示              │
│                               listen("remote-move")     │
└─────────────────────┬───────────────────────────────────┘
                      │ invoke / listen
┌─ Tauri IPC ─────────┼───────────────────────────────────┐
│  commands.rs         │                                  │
│  host_game(port)     │  对手落子 →                      │
│  join_game(ip,port)  │  app.emit("remote-move")         │
│  send_move(x,y)      │                                  │
│  send_undo()         │  AppState.network_tx             │
│  send_resign()       │  channel 发送端                  │
└─────────┬────────────┴──────────────────────────────────┘
          │ mpsc::channel (NetworkCmd / NetworkEvent)
┌─ 网络线程 ──────────────────────────────────────────────┐
│  NetworkLoop                                            │
│  Server: RenetServer + NetcodeServerTransport            │
│  Client: RenetClient + NetcodeClientTransport            │
│                                                          │
│  loop { update → recv → process → send_packets }        │
│  16ms 帧率，消息通过 channel 与 commands 层通信          │
└──────────────────────────────────────────────────────────┘
```

## 通信模型

renet 内置 Server/Client。主机运行 RenetServer，对手作为 RenetClient 连接。所有消息经主机转发。

### 消息协议

网络传输使用 serde JSON + renet ReliableOrdered 通道：

```rust
enum NetMessage {
    Move { x: usize, y: usize, turn: u32 },
    Undo  { steps: u32 },
    Resign,
}
```

### Channel 接口

```rust
// commands → 网络线程
enum NetworkCmd {
    SendMove { x: usize, y: usize, turn: u32 },
    SendUndo { steps: u32 },
    SendResign,
    Shutdown,
}

// 网络线程 → commands
enum NetworkEvent {
    RemoteMove { x: usize, y: usize },
    RemoteUndo { steps: u32 },
    RemoteResign,
    ClientConnected,
    ClientDisconnected,
    Error(String),
}
```

## 连接流程

```
主机 (Server)                          对手 (Client)
  host_game(port)
  绑定 UDP "0.0.0.0:{port}"
  NetworkLoop::Server 启动
  返回实际端口
  emit("waiting") ───────────────────→ 对手 join_game(ip, port)
                                        bind UDP "0.0.0.0:0"
                                        NetworkLoop::Client 启动
                                        connect to server ──→
                                        ← connected ─────────
  收到 ClientConnected
  emit("opponent-joined") ←──────────→ emit("connected")

  游戏开始，黑方(主机)先手正常落子
  place_piece → NetworkCmd::SendMove ─→ broadcast ─→ client recv
                                                       NetworkEvent::RemoteMove
                                                       invoke place_piece
```

## 生命周期

- 网络线程在 `new_game(Online)` 时 spawn
- 游戏结束或 AppState drop 时：发送 Shutdown → 线程退出 → join
- 对手断开：主机的 ClientDisconnected event → emit 对手获胜

## 前端改动

### OnlineSetup
- 创建房间：输入端口号（可选，默认随机），显示"我的地址: IP:端口"
- 加入房间：输入"IP:端口"，连接

### GameView — 新增连接状态条
- 等待中：显示"等待对手加入... (你的地址: IP:端口)"
- 已连接：显示"已连接"
- 已断开：显示"对手断开连接" + 对手获胜

### BoardCanvas — 监听 remote-move
```typescript
useEffect(() => {
  const unlisten = listen<{ x: number; y: number }>('remote-move', (event) => {
    placePiece(event.payload.x, event.payload.y);
  });
  return () => { unlisten.then(fn => fn()); };
}, []);
```

### GameControls
- Online 模式：不显示悔棋（需双方同意，暂不做）
- 认输：调用 send_resign

## AppState 改动

```rust
pub struct AppState {
    // ... 现有字段 ...
    pub network_tx: Mutex<Option<Sender<NetworkCmd>>>,
}
```

## 依赖

```toml
# core/Cargo.toml & gui/Cargo.toml
renet = { version = "0.0.23", features = ["netcode"] }
renet_netcode = "0.0.15"
serde_json = "1"  # 已有
```

## 不做 (YAGNI)

- Chat 功能（NetMessage 保留类型但无 UI）
- NAT 穿透 / 中转服务器
- 断线重连
- 观战模式
- 悔棋双方确认（Online 模式直接禁悔棋）

## 测试策略

- Rust 单元测试：NetMessage serde 往返、NetworkCmd/Event channel 通信、NetworkLoop::new 创建
- 集成测试：renet 本地 client 模拟（server.new_local_client）
- 前端测试：OnlineSetup 组件渲染、状态文本切换
- 手动测试：双窗口（host + join localhost）
