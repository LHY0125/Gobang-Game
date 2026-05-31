use serde::{Deserialize, Serialize};

/// 网络传输的游戏消息（bincode 序列化）
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
#[derive(Debug)]
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

/// 网络循环句柄（在独立线程中运行）
pub struct NetworkLoop {
    role: NetworkRole,
    running: bool,
    cmd_rx: std::sync::mpsc::Receiver<NetworkCmd>,
    event_tx: std::sync::mpsc::Sender<NetworkEvent>,
}

impl NetworkLoop {
    /// 创建 Server 端
    pub fn new_server(
        cmd_rx: std::sync::mpsc::Receiver<NetworkCmd>,
        event_tx: std::sync::mpsc::Sender<NetworkEvent>,
    ) -> Self {
        Self {
            role: NetworkRole::Server,
            running: false,
            cmd_rx,
            event_tx,
        }
    }

    /// 创建 Client 端
    pub fn new_client(
        cmd_rx: std::sync::mpsc::Receiver<NetworkCmd>,
        event_tx: std::sync::mpsc::Sender<NetworkEvent>,
    ) -> Self {
        Self {
            role: NetworkRole::Client,
            running: false,
            cmd_rx,
            event_tx,
        }
    }

    /// 启动网络主循环（阻塞，在独立线程中调用）
    pub fn run(&mut self, server_addr: &str, protocol_id: u64) -> Result<(), String> {
        self.running = true;
        match self.role {
            NetworkRole::Server => self.run_server(protocol_id),
            NetworkRole::Client => self.run_client(server_addr, protocol_id),
        }
    }

    fn run_server(&mut self, protocol_id: u64) -> Result<(), String> {
        use std::net::UdpSocket;
        use std::time::{Duration, Instant};

        let socket = UdpSocket::bind("0.0.0.0:0").map_err(|e| format!("Server 绑定失败: {}", e))?;
        let local_addr = socket.local_addr().map_err(|e| e.to_string())?;
        let local_port = local_addr.port();
        let _ = self.event_tx.send(NetworkEvent::Listening(local_port));

        let connection_config =
            renet2::ConnectionConfig::from_shared_channels(vec![renet2::ChannelConfig {
                channel_id: 0,
                max_memory_usage_bytes: 5 * 1024 * 1024,
                send_type: renet2::SendType::ReliableOrdered {
                    resend_time: Duration::from_millis(300),
                },
            }]);
        let mut server = renet2::RenetServer::new(connection_config);

        let current_time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default();
        let server_config = renet2_netcode::ServerSetupConfig {
            current_time,
            max_clients: 1,
            protocol_id,
            socket_addresses: vec![vec![local_addr]],
            authentication: renet2_netcode::ServerAuthentication::Unsecure,
        };
        let native_socket = renet2_netcode::NativeSocket::new(socket)
            .map_err(|e| format!("创建 NativeSocket 失败: {}", e))?;
        let mut transport =
            renet2_netcode::NetcodeServerTransport::new(server_config, native_socket)
                .map_err(|e| format!("创建传输层失败: {}", e))?;

        let tick = Duration::from_millis(16);

        while self.running {
            let now = Instant::now();

            // 处理 commands 层发来的指令
            while let Ok(cmd) = self.cmd_rx.try_recv() {
                match cmd {
                    NetworkCmd::SendMove { x, y, turn } => {
                        let msg = NetMessage::Move { x, y, turn };
                        for cid in server.clients_id() {
                            server.send_message(cid, 0u8, msg.to_bytes());
                        }
                    }
                    NetworkCmd::SendUndo { steps } => {
                        let msg = NetMessage::Undo { steps };
                        for cid in server.clients_id() {
                            server.send_message(cid, 0u8, msg.to_bytes());
                        }
                    }
                    NetworkCmd::SendResign => {
                        for cid in server.clients_id() {
                            server.send_message(cid, 0u8, NetMessage::Resign.to_bytes());
                        }
                    }
                    NetworkCmd::Shutdown => {
                        self.running = false;
                        break;
                    }
                }
            }

            server.update(tick);
            transport
                .update(tick, &mut server)
                .map_err(|e| format!("传输层更新失败: {e:?}"))?;

            // 接收客户端消息
            for cid in server.clients_id() {
                while let Some(data) = server.receive_message(cid, 0u8) {
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
                    renet2::ServerEvent::ClientConnected { .. } => {
                        let _ = self.event_tx.send(NetworkEvent::ClientConnected);
                    }
                    renet2::ServerEvent::ClientDisconnected { .. } => {
                        let _ = self.event_tx.send(NetworkEvent::ClientDisconnected);
                    }
                }
            }

            transport.send_packets(&mut server);

            let elapsed = now.elapsed();
            if elapsed < tick {
                std::thread::sleep(tick - elapsed);
            }
        }
        Ok(())
    }

    fn run_client(&mut self, server_addr: &str, protocol_id: u64) -> Result<(), String> {
        use std::net::{SocketAddr, UdpSocket};
        use std::time::{Duration, Instant};

        let server_addr: SocketAddr = server_addr
            .parse()
            .map_err(|e| format!("地址解析失败: {}", e))?;
        let socket = UdpSocket::bind("0.0.0.0:0").map_err(|e| format!("Client 绑定失败: {}", e))?;

        let connection_config =
            renet2::ConnectionConfig::from_shared_channels(vec![renet2::ChannelConfig {
                channel_id: 0,
                max_memory_usage_bytes: 5 * 1024 * 1024,
                send_type: renet2::SendType::ReliableOrdered {
                    resend_time: Duration::from_millis(300),
                },
            }]);
        let mut client = renet2::RenetClient::new(connection_config, false);

        let current_time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default();
        let authentication = renet2_netcode::ClientAuthentication::Unsecure {
            server_addr,
            client_id: current_time.as_millis() as u64,
            user_data: None,
            protocol_id,
            socket_id: 0,
        };
        let native_socket = renet2_netcode::NativeSocket::new(socket)
            .map_err(|e| format!("创建 NativeSocket 失败: {}", e))?;
        let mut transport = renet2_netcode::NetcodeClientTransport::new(
            current_time,
            authentication,
            native_socket,
        )
        .map_err(|e| format!("创建传输层失败: {}", e))?;

        let tick = Duration::from_millis(16);
        let mut was_connected = false;

        while self.running {
            let now = Instant::now();

            while let Ok(cmd) = self.cmd_rx.try_recv() {
                match cmd {
                    NetworkCmd::SendMove { x, y, turn } => {
                        let msg = NetMessage::Move { x, y, turn };
                        client.send_message(0u8, msg.to_bytes());
                    }
                    NetworkCmd::SendUndo { steps } => {
                        let msg = NetMessage::Undo { steps };
                        client.send_message(0u8, msg.to_bytes());
                    }
                    NetworkCmd::SendResign => {
                        client.send_message(0u8, NetMessage::Resign.to_bytes());
                    }
                    NetworkCmd::Shutdown => {
                        self.running = false;
                        break;
                    }
                }
            }

            client.update(tick);
            transport
                .update(tick, &mut client)
                .map_err(|e| format!("传输层更新失败: {e:?}"))?;

            if client.is_connected() && !was_connected {
                was_connected = true;
                let _ = self.event_tx.send(NetworkEvent::Connected);
            }
            if !client.is_connected() && was_connected {
                was_connected = false;
                let _ = self.event_tx.send(NetworkEvent::ClientDisconnected);
            }

            while let Some(data) = client.receive_message(0u8) {
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

            transport
                .send_packets(&mut client)
                .map_err(|e| format!("发送数据包失败: {e}"))?;

            let elapsed = now.elapsed();
            if elapsed < tick {
                std::thread::sleep(tick - elapsed);
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_net_message_move_roundtrip() {
        let msg = NetMessage::Move {
            x: 7,
            y: 7,
            turn: 0,
        };
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

    #[test]
    fn test_network_cmd_channel() {
        let (tx, rx) = std::sync::mpsc::channel();
        tx.send(NetworkCmd::SendMove {
            x: 7,
            y: 7,
            turn: 0,
        })
        .unwrap();
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
}
