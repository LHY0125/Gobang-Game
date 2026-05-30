use serde::{Deserialize, Serialize};

/// 游戏网络消息
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum GameMessage {
    Move { x: usize, y: usize, turn: u32 },
    Undo { steps: u32 },
    Resign,
    Chat(String),
    Heartbeat,
}

/// 网络连接角色
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NetworkRole {
    Server,
    Client,
}

/// 网络会话配置
#[derive(Debug, Clone)]
pub struct NetworkConfig {
    pub role: NetworkRole,
    pub bind_port: u16,
    pub remote_addr: String,
    pub remote_port: u16,
}

/// 网络会话状态
#[derive(Debug, Clone)]
pub struct NetworkSession {
    pub role: NetworkRole,
    pub is_connected: bool,
    pub config: NetworkConfig,
    pending_messages: Vec<GameMessage>,
}

impl NetworkSession {
    pub fn new(config: NetworkConfig) -> Self {
        Self {
            role: config.role,
            is_connected: false,
            config,
            pending_messages: Vec::new(),
        }
    }

    /// 发送消息 (实际 renet 集成在 gui 层处理)
    pub fn enqueue_message(&mut self, msg: GameMessage) {
        self.pending_messages.push(msg);
    }

    /// 取出待发送的消息
    pub fn drain_messages(&mut self) -> Vec<GameMessage> {
        std::mem::take(&mut self.pending_messages)
    }

    pub fn set_connected(&mut self, connected: bool) {
        self.is_connected = connected;
    }
}
