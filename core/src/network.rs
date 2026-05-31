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
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_net_message_move_roundtrip() {
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
}
