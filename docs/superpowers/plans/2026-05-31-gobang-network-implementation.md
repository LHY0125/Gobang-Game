# Gobang 网络对战实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现基于 renet UDP 直连的 P2P 网络对战，支持创建房间/加入房间/同步落子/悔棋/认输。

**Architecture:** 独立网络线程跑 renet Server/Client 循环，通过 mpsc channel 与 Tauri commands 层通信，对手落子通过 Tauri `app.emit("remote-move")` 事件推送到前端。

**Tech Stack:** renet 2.0, renet_netcode, mpsc channel, Tauri event system, React + Zustand

---

## 文件变更总览

| 文件 | 操作 | 内容 |
|------|------|------|
| `core/Cargo.toml` | 修改 | 加 renet, renet_netcode, bincode |
| `core/src/network.rs` | 重写 | NetworkLoop struct, NetMessage, channel 类型 |
| `core/src/types.rs` | 修改 | GameConfig 加 host_port |
| `gui/Cargo.toml` | 修改 | 加 renet, renet_netcode |
| `gui/src/commands.rs` | 修改 | host_game, join_game, send_move, send_undo, send_resign, AppState.network_tx |
| `gui/src/lib.rs` | 修改 | 注册 5 个新命令 |
| `src/core/types.ts` | 修改 | GameConfig 加 hostPort |
| `src/components/menu/OnlineSetup.tsx` | 重写 | 创建房间显示地址 + 加入房间输入 |
| `src/components/menu/MainMenu.tsx` | 修改 | 启用 Online 按钮 |
| `src/components/game/GameView.tsx` | 修改 | 加连接状态条 |
| `src/components/game/GameControls.tsx` | 修改 | Online 模式禁悔棋 |
| `src/components/board/BoardCanvas.tsx` | 修改 | 监听 remote-move event |
| `src/store/gameStore.ts` | 修改 | remote-move 处理 |
| `src/i18n/zh-CN.json` | 修改 | 加连接状态 key |
| `src/i18n/en.json` | 修改 | 加连接状态 key |

---

### Task 1: 添加 renet 依赖

**Files:**
- Modify: `core/Cargo.toml`
- Modify: `gui/Cargo.toml`

- [ ] **Step 1: 在 core/Cargo.toml 添加依赖**

在 `[dependencies]` 下添加：

```toml
renet2 = "2"
bincode = "1"
```

`bincode` 用于将 NetMessage 序列化为二进制（比 JSON 更紧凑，适合游戏网络包）。

- [ ] **Step 2: 在 gui/Cargo.toml 添加依赖**

在 `[dependencies]` 下添加：

```toml
renet2 = "2"
```

（gui 层需要 renet2 来创建网络线程中的 RenetServer/RenetClient）

- [ ] **Step 3: 验证依赖解析**

```bash
cargo check
```

Expected: 依赖下载成功，编译通过。

- [ ] **Step 4: 提交**

```bash
git add core/Cargo.toml gui/Cargo.toml
git commit -m "chore: 添加 renet2 + bincode 网络库依赖"
```

---

### Task 2: 重写 core/src/network.rs

**Files:**
- Modify: `core/src/network.rs` (完全重写)

- [ ] **Step 1: 定义 NetMessage 和 channel 类型**

用以下内容替换 `core/src/network.rs`：

```rust
use serde::{Deserialize, Serialize};

/// 网络传输的游戏消息
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum NetMessage {
    Move { x: usize, y: usize, turn: u32 },
    Undo { steps: u32 },
    Resign,
}

impl NetMessage {
    pub fn to_bytes(&self) -> Vec<u8> {
        bincode::serialize(self).unwrap_or_default()
    }

    pub fn from_bytes(data: &[u8]) -> Option<Self> {
        bincode::deserialize(data).ok()
    }
}

/// commands 层 → 网络线程
pub enum NetworkCmd {
    SendMove { x: usize, y: usize, turn: u32 },
    SendUndo { steps: u32 },
    SendResign,
    Shutdown,
}

/// 网络线程 → commands 层
#[derive(Debug, Clone)]
pub enum NetworkEvent {
    RemoteMove { x: usize, y: usize },
    RemoteUndo { steps: u32 },
    RemoteResign,
    ClientConnected,
    ClientDisconnected,
    Error(String),
    Listening(u16),
    Connected,
}

/// 网络角色
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NetworkRole {
    Server,
    Client,
}
```

- [ ] **Step 2: 定义 NetworkLoop struct 和构造**

在文件末尾追加：

```rust
use std::net::UdpSocket;
use std::sync::mpsc;
use std::time::Duration;

pub struct NetworkLoop {
    role: NetworkRole,
    running: bool,
    cmd_rx: mpsc::Receiver<NetworkCmd>,
    event_tx: mpsc::Sender<NetworkEvent>,
}

impl NetworkLoop {
    /// 创建 Server 端 NetworkLoop
    pub fn new_server(
        port: u16,
        cmd_rx: mpsc::Receiver<NetworkCmd>,
        event_tx: mpsc::Sender<NetworkEvent>,
    ) -> Result<(Self, u16), String> {
        let socket = UdpSocket::bind(format!("0.0.0.0:{}", port))
            .map_err(|e| format!("绑定端口失败: {}", e))?;
        let actual_port = socket.local_addr().map_err(|e| e.to_string())?.port();
        Ok((
            Self {
                role: NetworkRole::Server,
                running: false,
                cmd_rx,
                event_tx,
            },
            actual_port,
        ))
    }

    /// 创建 Client 端 NetworkLoop
    pub fn new_client(
        cmd_rx: mpsc::Receiver<NetworkCmd>,
        event_tx: mpsc::Sender<NetworkEvent>,
    ) -> Self {
        Self {
            role: NetworkRole::Client,
            running: false,
            cmd_rx,
            event_tx,
        }
    }
}
```

- [ ] **Step 3: 写 NetworkMessage serde 往返测试**

在文件末尾 `#[cfg(test)]` 模块中：

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_net_message_roundtrip() {
        let msg = NetMessage::Move { x: 7, y: 7, turn: 0 };
        let bytes = msg.to_bytes();
        let decoded = NetMessage::from_bytes(&bytes).unwrap();
        match decoded {
            NetMessage::Move { x, y, turn } => {
                assert_eq!(x, 7);
                assert_eq!(y, 7);
                assert_eq!(turn, 0);
            }
            _ => panic!("wrong variant"),
        }
    }

    #[test]
    fn test_net_message_undo_roundtrip() {
        let msg = NetMessage::Undo { steps: 1 };
        let bytes = msg.to_bytes();
        let decoded = NetMessage::from_bytes(&bytes).unwrap();
        assert!(matches!(decoded, NetMessage::Undo { steps: 1 }));
    }

    #[test]
    fn test_net_message_resign_roundtrip() {
        let msg = NetMessage::Resign;
        let bytes = msg.to_bytes();
        let decoded = NetMessage::from_bytes(&bytes).unwrap();
        assert!(matches!(decoded, NetMessage::Resign));
    }
}
```

- [ ] **Step 4: 验证编译和测试**

```bash
cargo test -p gobang-core
```

Expected: 3 个新测试 + 27 个已有测试全部通过。

- [ ] **Step 5: 提交**

```bash
git add core/src/network.rs
git commit -m "feat: 重写 network.rs — NetMessage/NetworkCmd/NetworkEvent 定义 + serde 测试"
```

---

### Task 3: 实现 NetworkLoop run 方法（主循环）

**Files:**
- Modify: `core/src/network.rs` (追加 run 方法)

- [ ] **Step 1: 实现 NetworkLoop::run()**

在 `NetworkLoop` 的 `impl` 块中追加 `run` 方法：

```rust
/// 启动网络主循环（在独立线程中调用）
pub fn run(&mut self, server_addr: &str, protocol_id: u64) -> Result<(), String> {
    self.running = true;

    match self.role {
        NetworkRole::Server => self.run_server(protocol_id),
        NetworkRole::Client => self.run_client(server_addr, protocol_id),
    }
}

fn run_server(&mut self, protocol_id: u64) -> Result<(), String> {
    let socket = UdpSocket::bind("0.0.0.0:0")
        .map_err(|e| format!("绑定失败: {}", e))?;
    let local_port = socket.local_addr().map_err(|e| e.to_string())?.port();
    let _ = self.event_tx.send(NetworkEvent::Listening(local_port));

    let mut server = renet2::RenetServer::new(renet2::ConnectionConfig {
        available_bytes_per_tick: 1024,
        server_channels_config: vec![],
        client_channels_config: vec![renet2::ChannelConfig {
            channel_id: 0,
            send_type: renet2::SendType::ReliableOrdered { resend_time: Duration::from_secs(1) },
        }],
    });

    let server_config = renet2::ServerConfig {
        current_time: std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default(),
        max_clients: 1,
        protocol_id,
        authentication: renet2::ServerAuthentication::Unsecure,
    };
    let mut transport = renet2::NetcodeServerTransport::new(server_config, socket)
        .map_err(|e| format!("创建传输层失败: {}", e))?;

    let mut client_id: Option<renet2::ClientId> = None;
    let tick = Duration::from_millis(16);

    while self.running {
        let now = std::time::Instant::now();

        // 处理 channel 指令
        while let Ok(cmd) = self.cmd_rx.try_recv() {
            match cmd {
                NetworkCmd::SendMove { x, y, turn } => {
                    let msg = NetMessage::Move { x, y, turn };
                    if let Some(cid) = client_id {
                        server.send_message(cid, 0, msg.to_bytes().into());
                    }
                }
                NetworkCmd::SendUndo { steps } => {
                    let msg = NetMessage::Undo { steps };
                    if let Some(cid) = client_id {
                        server.send_message(cid, 0, msg.to_bytes().into());
                    }
                }
                NetworkCmd::SendResign => {
                    let msg = NetMessage::Resign;
                    if let Some(cid) = client_id {
                        server.send_message(cid, 0, msg.to_bytes().into());
                    }
                }
                NetworkCmd::Shutdown => {
                    self.running = false;
                    break;
                }
            }
        }

        // 更新
        server.update(tick);
        transport.update(tick, &mut server)
            .map_err(|e| format!("传输更新失败: {}", e))?;

        // 接收消息
        if let Some(cid) = client_id {
            while let Some(data) = server.receive_message(cid, 0) {
                if let Some(msg) = NetMessage::from_bytes(&data) {
                    match msg {
                        NetMessage::Move { x, y, .. } => {
                            let _ = self.event_tx.send(NetworkEvent::RemoteMove { x, y });
                        }
                        NetMessage::Undo { steps } => {
                            let _ = self.event_tx.send(NetworkEvent::RemoteUndo { steps });
                        }
                        NetMessage::Resign => {
                            let _ = self.event_tx.send(NetworkEvent::RemoteResign);
                        }
                    }
                }
            }
        }

        // 处理连接事件
        while let Some(event) = server.get_event() {
            match event {
                renet2::ServerEvent::ClientConnected { client_id: cid } => {
                    client_id = Some(cid);
                    let _ = self.event_tx.send(NetworkEvent::ClientConnected);
                }
                renet2::ServerEvent::ClientDisconnected { .. } => {
                    client_id = None;
                    let _ = self.event_tx.send(NetworkEvent::ClientDisconnected);
                }
            }
        }

        transport.send_packets(&mut server)
            .map_err(|e| format!("发送失败: {}", e))?;

        let elapsed = now.elapsed();
        if elapsed < tick {
            std::thread::sleep(tick - elapsed);
        }
    }

    Ok(())
}

fn run_client(&mut self, server_addr: &str, protocol_id: u64) -> Result<(), String> {
    let server_addr: std::net::SocketAddr = server_addr
        .parse()
        .map_err(|e| format!("地址解析失败: {}", e))?;
    let socket = UdpSocket::bind("0.0.0.0:0")
        .map_err(|e| format!("绑定失败: {}", e))?;

    let mut client = renet2::RenetClient::new(renet2::ConnectionConfig {
        available_bytes_per_tick: 1024,
        server_channels_config: vec![],
        client_channels_config: vec![renet2::ChannelConfig {
            channel_id: 0,
            send_type: renet2::SendType::ReliableOrdered { resend_time: Duration::from_secs(1) },
        }],
    });

    let current_time = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap_or_default();
    let authentication = renet2::ClientAuthentication::Unsecure {
        protocol_id,
        server_addr,
        client_id: current_time.as_millis() as u64,
        user_data: None,
    };
    let mut transport = renet2::NetcodeClientTransport::new(current_time, authentication, socket)
        .map_err(|e| format!("创建传输层失败: {}", e))?;

    let tick = Duration::from_millis(16);
    let mut was_connected = false;

    while self.running {
        let now = std::time::Instant::now();

        // 处理指令
        while let Ok(cmd) = self.cmd_rx.try_recv() {
            match cmd {
                NetworkCmd::SendMove { x, y, turn } => {
                    let msg = NetMessage::Move { x, y, turn };
                    client.send_message(0, msg.to_bytes().into());
                }
                NetworkCmd::SendUndo { steps } => {
                    let msg = NetMessage::Undo { steps };
                    client.send_message(0, msg.to_bytes().into());
                }
                NetworkCmd::SendResign => {
                    let msg = NetMessage::Resign;
                    client.send_message(0, msg.to_bytes().into());
                }
                NetworkCmd::Shutdown => {
                    self.running = false;
                    break;
                }
            }
        }

        client.update(tick);
        transport.update(tick, &mut client)
            .map_err(|e| format!("传输更新失败: {}", e))?;

        // 连接状态变化
        if client.is_connected() && !was_connected {
            was_connected = true;
            let _ = self.event_tx.send(NetworkEvent::Connected);
        }
        if !client.is_connected() && was_connected {
            was_connected = false;
            let _ = self.event_tx.send(NetworkEvent::ClientDisconnected);
        }

        // 接收消息
        while let Some(data) = client.receive_message(0) {
            if let Some(msg) = NetMessage::from_bytes(&data) {
                match msg {
                    NetMessage::Move { x, y, .. } => {
                        let _ = self.event_tx.send(NetworkEvent::RemoteMove { x, y });
                    }
                    NetMessage::Undo { steps } => {
                        let _ = self.event_tx.send(NetworkEvent::RemoteUndo { steps });
                    }
                    NetMessage::Resign => {
                        let _ = self.event_tx.send(NetworkEvent::RemoteResign);
                    }
                }
            }
        }

        transport.send_packets(&mut client)
            .map_err(|e| format!("发送失败: {}", e))?;

        let elapsed = now.elapsed();
        if elapsed < tick {
            std::thread::sleep(tick - elapsed);
        }
    }

    Ok(())
}
```

- [ ] **Step 2: 验证编译**

```bash
cargo check -p gobang-core
```

Expected: 编译通过（可能需要根据实际 renet2 API 微调方法名）。

- [ ] **Step 3: 提交**

```bash
git add core/src/network.rs
git commit -m "feat: 实现 NetworkLoop::run — Server/Client 主循环"
```

---

### Task 4: GameConfig 添加 host_port 字段

**Files:**
- Modify: `core/src/types.rs`
- Modify: `src/core/types.ts`

- [ ] **Step 1: Rust GameConfig 加 host_port**

在 `core/src/types.rs` 的 `GameConfig` struct 末尾（`remote_address` 之后）添加：

```rust
#[serde(default)]
pub host_port: u16,
```

在 `Default` impl 中对应位置添加：

```rust
host_port: 0,
```

- [ ] **Step 2: TypeScript GameConfig 加 hostPort**

在 `src/core/types.ts` 的 `GameConfig` interface 末尾添加：

```typescript
  hostPort: number;
```

- [ ] **Step 3: 验证**

```bash
cargo check
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add core/src/types.rs src/core/types.ts
git commit -m "feat: GameConfig 新增 hostPort 字段"
```

---

### Task 5: gui 层网络命令 + AppState.network_tx

**Files:**
- Modify: `gui/src/commands.rs`

- [ ] **Step 1: 添加 network_tx 到 AppState**

在 `gui/src/commands.rs` 的 `AppState` struct 中现有字段末尾添加：

```rust
pub network_tx: Mutex<Option<std::sync::mpsc::Sender<gobang_core::network::NetworkCmd>>>,
```

在 `Default` impl 中添加：

```rust
network_tx: Mutex::new(None),
```

- [ ] **Step 2: 添加 import**

在文件顶部添加：

```rust
use gobang_core::network::{NetworkCmd, NetworkEvent, NetworkLoop, NetMessage};
use std::sync::mpsc;
```

- [ ] **Step 3: 添加 host_game 命令**

```rust
#[tauri::command]
pub fn host_game(port: u16, state: State<AppState>, app: tauri::AppHandle) -> Result<u16, String> {
    let (cmd_tx, cmd_rx) = mpsc::channel();
    let (event_tx, event_rx) = mpsc::channel();

    let (mut network, actual_port) = NetworkLoop::new_server(port, cmd_rx, event_tx)?;

    *state.network_tx.lock().map_err(|e| e.to_string())? = Some(cmd_tx);

    // spawn 网络线程
    let protocol_id: u64 = 7777;
    std::thread::spawn(move || {
            let _ = network.run("", protocol_id);
    });

    // spawn event 转发线程 → Tauri events
    let app_clone = app.clone();
    std::thread::spawn(move || {
        for event in event_rx {
            match event {
                NetworkEvent::RemoteMove { x, y } => {
                    let _ = app_clone.emit("remote-move", serde_json::json!({ "x": x, "y": y }));
                }
                NetworkEvent::RemoteUndo { steps } => {
                    let _ = app_clone.emit("remote-undo", steps);
                }
                NetworkEvent::RemoteResign => {
                    let _ = app_clone.emit("remote-resign", ());
                }
                NetworkEvent::ClientConnected | NetworkEvent::Connected => {
                    let _ = app_clone.emit("connection-status", "connected");
                }
                NetworkEvent::ClientDisconnected => {
                    let _ = app_clone.emit("connection-status", "disconnected");
                }
                NetworkEvent::Listening(port) => {
                    let _ = app_clone.emit("listening-port", port);
                }
                NetworkEvent::Error(msg) => {
                    let _ = app_clone.emit("network-error", msg);
                }
            }
        }
    });

    Ok(actual_port)
}
```

- [ ] **Step 4: 添加 join_game 命令**

```rust
#[tauri::command]
pub fn join_game(address: String, state: State<AppState>, app: tauri::AppHandle) -> Result<(), String> {
    let (cmd_tx, cmd_rx) = mpsc::channel();
    let (event_tx, event_rx) = mpsc::channel();

    let mut network = NetworkLoop::new_client(cmd_rx, event_rx);

    *state.network_tx.lock().map_err(|e| e.to_string())? = Some(cmd_tx);

    let protocol_id: u64 = 7777;
    let addr = address.clone();
    std::thread::spawn(move || {
        let _ = network.run(&addr, protocol_id);
    });

    let app_clone = app.clone();
    std::thread::spawn(move || {
        for event in event_rx {
            match event {
                NetworkEvent::RemoteMove { x, y } => {
                    let _ = app_clone.emit("remote-move", serde_json::json!({ "x": x, "y": y }));
                }
                NetworkEvent::RemoteUndo { steps } => {
                    let _ = app_clone.emit("remote-undo", steps);
                }
                NetworkEvent::RemoteResign => {
                    let _ = app_clone.emit("remote-resign", ());
                }
                NetworkEvent::Connected => {
                    let _ = app_clone.emit("connection-status", "connected");
                }
                NetworkEvent::ClientDisconnected => {
                    let _ = app_clone.emit("connection-status", "disconnected");
                }
                NetworkEvent::Error(msg) => {
                    let _ = app_clone.emit("network-error", msg);
                }
                _ => {}
            }
        }
    });

    Ok(())
}
```

- [ ] **Step 5: 添加 send_move/send_undo/send_resign 命令**

```rust
#[tauri::command]
pub fn send_move(x: usize, y: usize, turn: u32, state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendMove { x, y, turn }).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn send_undo(steps: u32, state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendUndo { steps }).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn send_resign(state: State<AppState>) -> Result<(), String> {
    let tx = state.network_tx.lock().map_err(|e| e.to_string())?;
    let tx = tx.as_ref().ok_or("未建立网络连接")?;
    tx.send(NetworkCmd::SendResign).map_err(|e| e.to_string())
}
```

- [ ] **Step 6: 在 new_game 中初始化/清理 network_tx**

在 `new_game` 中，游戏模式为 Online 时不做额外处理（由 host_game/join_game 单独调用）。在 `new_game` 开头清理旧的 network_tx：

```rust
// 清理旧网络连接
if let Ok(mut tx) = state.network_tx.lock() {
    if let Some(tx) = tx.take() {
        let _ = tx.send(NetworkCmd::Shutdown);
    }
}
```

- [ ] **Step 7: 验证编译**

```bash
cargo check -p gobang-gui
```

Expected: 编译通过。renet2 API 可能需要根据实际路径调整。

- [ ] **Step 8: 提交**

```bash
git add gui/src/commands.rs
git commit -m "feat: 添加 host_game/join_game/send_move/send_undo/send_resign 命令"
```

---

### Task 6: 注册新命令 + 启用 Online 按钮

**Files:**
- Modify: `gui/src/lib.rs`
- Modify: `src/components/menu/MainMenu.tsx`

- [ ] **Step 1: 在 lib.rs 注册 5 个新命令**

在 `gui/src/lib.rs` 的 `generate_handler!` 中添加：

```rust
.invoke_handler(tauri::generate_handler![
    commands::new_game,
    commands::place_piece,
    commands::undo,
    commands::ai_move,
    commands::get_game_state,
    commands::resign,
    commands::save_record,
    commands::host_game,
    commands::join_game,
    commands::send_move,
    commands::send_undo,
    commands::send_resign,
])
```

- [ ] **Step 2: 启用 MainMenu 的 Online 按钮**

在 `src/components/menu/MainMenu.tsx` 中，移除 Online 按钮的 `disabled` 属性，去掉 `(开发中)` 后缀：

```tsx
<button onClick={() => setView('online')}>
  {t('menu.online_game')}
</button>
```

- [ ] **Step 3: 验证**

```bash
cargo check
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add gui/src/lib.rs src/components/menu/MainMenu.tsx
git commit -m "feat: 注册网络命令 + 启用 Online 按钮"
```

---

### Task 7: 重写 OnlineSetup + 更新 GameView/BoardCanvas/GameControls

**Files:**
- Modify: `src/components/menu/OnlineSetup.tsx` (重写)
- Modify: `src/components/game/GameView.tsx`
- Modify: `src/components/board/BoardCanvas.tsx`
- Modify: `src/components/game/GameControls.tsx`

- [ ] **Step 1: 重写 OnlineSetup.tsx**

```tsx
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { GameConfig } from '../../core/types';

interface Props { onBack: () => void; onStart: () => void; }

export default function OnlineSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [boardSize, setBoardSize] = useState(15);
  const [ip, setIp] = useState('');
  const [myAddress, setMyAddress] = useState('');
  const [isHosting, setIsHosting] = useState(false);

  const handleHost = async () => {
    const port: number = await invoke('host_game', { port: 0 });
    setMyAddress(`127.0.0.1:${port}`);
    setIsHosting(true);

    // 等待对手连接
    const unlisten = await listen<string>('connection-status', async (event) => {
      if (event.payload === 'connected') {
        unlisten();
        const config: GameConfig = {
          boardSize, useForbiddenRules: true, useTimer: false,
          timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'Black',
          isServer: true, remoteAddress: '', hostPort: port,
        };
        await startGame('Online', config);
        onStart();
      }
    });
  };

  const handleJoin = async () => {
    const [host, portStr] = ip.split(':');
    const config: GameConfig = {
      boardSize, useForbiddenRules: true, useTimer: false,
      timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'White',
      isServer: false, remoteAddress: ip, hostPort: parseInt(portStr) || 0,
    };
    await startGame('Online', config);
    await invoke('join_game', { address: ip });
    onStart();
  };

  if (isHosting) {
    return (
      <div className="setup-panel">
        <h2>{t('menu.online_game')}</h2>
        <p style={{ fontSize: 18 }}>等待对手加入...</p>
        <p style={{ fontSize: 24, fontFamily: 'monospace', background: '#F5DEB3', color: '#3C2415', padding: '8px 16px', borderRadius: 4 }}>
          {myAddress}
        </p>
        <p style={{ fontSize: 14, opacity: 0.7 }}>将此地址发给对手</p>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    );
  }

  return (
    <div className="setup-panel">
      <h2>{t('menu.online_game')}</h2>
      <label>
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}×{s}</option>
          ))}
        </select>
      </label>
      <button onClick={handleHost}>{t('menu.host_room')}</button>
      <div style={{ display: 'flex', gap: 8, marginTop: 12 }}>
        <input value={ip} onChange={(e) => setIp(e.target.value)} placeholder={t('menu.ip_placeholder')} />
        <button onClick={handleJoin} disabled={!ip}>{t('menu.join_room')}</button>
      </div>
      <button onClick={onBack} style={{ marginTop: 12 }}>{t('common.back')}</button>
    </div>
  );
}
```

- [ ] **Step 2: 修改 GameView.tsx — 添加连接状态条**

```tsx
import { useState, useEffect } from 'react';
import { listen } from '@tauri-apps/api/event';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import BoardCanvas from '../board/BoardCanvas';
import GameInfo from './GameInfo';
import GameControls from './GameControls';
import TimerDisplay from './TimerDisplay';

interface Props {
  onBackToMenu: () => void;
}

export default function GameView({ onBackToMenu }: Props) {
  const { t } = useTranslation();
  const mode = useGameStore((s) => s.mode);
  const [connStatus, setConnStatus] = useState<string>('');

  useEffect(() => {
    if (mode !== 'Online') return;
    const init = async () => {
      const u1 = await listen<string>('connection-status', (e) => setConnStatus(e.payload));
      const u2 = await listen<string>('listening-port', (e) => setConnStatus(`waiting:${e.payload}`));
      return () => { u1(); u2(); };
    };
    init();
  }, [mode]);

  return (
    <div className="game-view">
      {mode === 'Online' && connStatus && (
        <div style={{ fontSize: 14, opacity: 0.8 }}>
          {connStatus.startsWith('waiting') ? '等待对手加入...' :
           connStatus === 'connected' ? t('game.opponent_connected') :
           connStatus === 'disconnected' ? t('game.opponent_disconnected') : ''}
        </div>
      )}
      <GameInfo />
      <div className="board-container">
        <BoardCanvas />
      </div>
      <TimerDisplay />
      <GameControls onBackToMenu={onBackToMenu} />
    </div>
  );
}
```

- [ ] **Step 3: 修改 BoardCanvas.tsx — 监听 remote-move**

在 `BoardCanvas` 组件的 `useEffect` 或新 `useEffect` 中添加 event listener：

```tsx
// 在组件中添加（import { listen } from '@tauri-apps/api/event'）
useEffect(() => {
  if (mode !== 'Online') return;
  let unlisten: (() => void) | undefined;

  const setup = async () => {
    unlisten = await listen<{ x: number; y: number }>('remote-move', (event) => {
      placePiece(event.payload.x, event.payload.y);
    });
  };
  setup();

  return () => { unlisten?.(); };
}, [mode, placePiece]);
```

- [ ] **Step 4: 修改 GameControls.tsx — Online 禁悔棋**

在 `GameControls` 中，获取 `mode` 状态，悔棋按钮在 Online 模式下禁用：

```tsx
const mode = useGameStore((s) => s.mode);

// 悔棋按钮:
<button onClick={handleUndo} disabled={status === 'game_over' || mode === 'Online'}>
  {t('game.undo')}
</button>
```

- [ ] **Step 5: 验证**

```bash
npx tsc -b
```

- [ ] **Step 6: 提交**

```bash
git add src/components/menu/OnlineSetup.tsx src/components/game/GameView.tsx src/components/board/BoardCanvas.tsx src/components/game/GameControls.tsx
git commit -m "feat: Online 模式 UI — 房间管理/连接状态/remote-move 监听/禁悔棋"
```

---

### Task 8: 更新 i18n + gameStore

**Files:**
- Modify: `src/i18n/zh-CN.json`
- Modify: `src/i18n/en.json`
- Modify: `src/store/gameStore.ts`

- [ ] **Step 1: 添加 i18n key**

在 `zh-CN.json` 的 `game` 块中添加：

```json
"opponent_connected": "对手已连接",
"opponent_disconnected": "对手已断开",
```

在 `en.json` 的 `game` 块中添加：

```json
"opponent_connected": "Opponent Connected",
"opponent_disconnected": "Opponent Disconnected",
```

- [ ] **Step 2: gameStore 的 startGame 适配 Online 模式**

确保 `startGame` 对 `Online` 模式设置正确的初始 currentColor（主机黑=Black，对手白=White）。

当前逻辑已通过 `config.playerColor` 设置，无需改动。

但 `placePiece` 中需要处理 Online 模式：当本地玩家落子后，调用 `send_move` 同步给对手。

在 `gameStore.ts` 的 `placePiece` 成功返回后添加：

```typescript
if (get().mode === 'Online' && !result.is_win) {
  const lastMove = get().moves[get().moves.length - 1];
  if (lastMove) {
    await invoke('send_move', { x: lastMove.position.x, y: lastMove.position.y, turn: lastMove.turn });
  }
}
```

- [ ] **Step 3: 验证**

```bash
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add src/i18n/zh-CN.json src/i18n/en.json src/store/gameStore.ts
git commit -m "feat: i18n 连接状态翻译 + gameStore Online send_move 同步"
```

---

### Task 9: 集成测试 + 构建验证

**Files:**
- Modify: `core/src/network.rs` (追加 local client 测试)

- [ ] **Step 1: 添加 renet 本地 client 集成测试**

在 `core/src/network.rs` 的 tests 模块中追加：

```rust
#[test]
fn test_net_message_all_variants_roundtrip() {
    let messages = vec![
        NetMessage::Move { x: 0, y: 0, turn: 1 },
        NetMessage::Move { x: 14, y: 14, turn: 42 },
        NetMessage::Undo { steps: 1 },
        NetMessage::Undo { steps: 2 },
        NetMessage::Resign,
    ];
    for msg in messages {
        let bytes = msg.to_bytes();
        let decoded = NetMessage::from_bytes(&bytes);
        assert!(decoded.is_some());
    }
}

#[test]
fn test_network_cmd_channel() {
    let (tx, rx) = std::sync::mpsc::channel();
    tx.send(NetworkCmd::SendMove { x: 7, y: 7, turn: 0 }).unwrap();
    tx.send(NetworkCmd::Shutdown).unwrap();

    let mut received = Vec::new();
    while let Ok(cmd) = rx.try_recv() {
        match cmd {
            NetworkCmd::Shutdown => break,
            NetworkCmd::SendMove { x, y, turn } => received.push((x, y, turn)),
            _ => {}
        }
    }
    assert_eq!(received, vec![(7, 7, 0)]);
}
```

- [ ] **Step 2: 运行全部测试**

```bash
cargo test
npx vitest run
```

Expected: Rust 32+ passed, vitest 10 passed。

- [ ] **Step 3: 构建**

```bash
cargo check && cargo clippy -- -D warnings
npx tsc -b
```

- [ ] **Step 4: 提交**

```bash
git add core/src/network.rs
git commit -m "test: NetMessage 全变体 + NetworkCmd channel 集成测试"
```

---

### Task 10: 最终验证 + 手动测试

- [ ] **Step 1: 完整测试套件**

```bash
cargo test
cargo clippy -- -D warnings
npx tsc -b
npx vitest run
```

Expected: 全部通过。

- [ ] **Step 2: 构建打包**

```bash
npx tauri build
```

Expected: NSIS 安装包成功生成。

- [ ] **Step 3: 手动测试**
  - 窗口1：创建房间，记录 IP:端口
  - 窗口2：加入房间，填入窗口1的地址
  - 验证：双方落子互相同步

---

## 执行顺序

```
Task 1 (依赖) → Task 2 (核心类型) → Task 3 (网络循环) → Task 4 (GameConfig)
                                                              ↓
                                              Task 5 (gui 命令) → Task 6 (注册+启用)
                                                                      ↓
                                              Task 7 (UI 重写) → Task 8 (i18n+store)
                                                                      ↓
                                                              Task 9 (测试)
                                                                      ↓
                                                              Task 10 (最终验证)
```
