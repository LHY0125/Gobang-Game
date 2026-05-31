use crate::ai::AiEngine;
use crate::board::Board;
use crate::types::{CellState, Color, Position};

/// 大模型 AI — 通过 HTTP API 调用
pub struct LlmAi {
    endpoint: String,
    api_key: String,
    model: String,
}

impl LlmAi {
    pub fn new(endpoint: &str, api_key: &str, model: &str) -> Self {
        Self {
            endpoint: endpoint.to_string(),
            api_key: api_key.to_string(),
            model: model.to_string(),
        }
    }

    /// 将棋盘序列化为 prompt
    pub fn board_to_prompt(board: &Board, color: Color) -> String {
        let mut s = String::from("你是一位五子棋高手。当前棋盘状态(0=空,1=黑,2=白):\n");
        for x in 0..board.size {
            for y in 0..board.size {
                let ch = match board.get(Position::new(x, y)) {
                    CellState::Empty => '0',
                    CellState::Occupied(Color::Black) => '1',
                    CellState::Occupied(Color::White) => '2',
                };
                s.push(ch);
                s.push(' ');
            }
            s.push('\n');
        }
        let color_str = match color {
            Color::Black => "黑棋(1)",
            Color::White => "白棋(2)",
        };
        s.push_str(&format!(
            "\n你是{}, 请返回最佳落子坐标 (格式: x,y)",
            color_str
        ));
        s
    }

    /// 解析 LLM 响应中的坐标
    pub fn parse_response(response: &str) -> Option<Position> {
        let bytes = response.as_bytes();
        for (i, &b) in bytes.iter().enumerate() {
            if b != b',' {
                continue;
            }
            // 从逗号向前找数字起始
            let x_start = (0..i)
                .rev()
                .take_while(|&j| bytes[j].is_ascii_digit())
                .last()
                .unwrap_or(i);
            if x_start == i {
                continue;
            }
            // 从逗号向后跳过空格找数字起始
            let y_start = (i + 1..bytes.len()).find(|&j| bytes[j].is_ascii_digit())?;
            // 从 y_start 向后找数字结束
            let y_end = (y_start + 1..bytes.len())
                .find(|&j| !bytes[j].is_ascii_digit())
                .unwrap_or(bytes.len());

            let x_str = &response[x_start..i];
            let y_str = &response[y_start..y_end];
            if let (Ok(x), Ok(y)) = (x_str.parse::<usize>(), y_str.parse::<usize>()) {
                return Some(Position::new(x, y));
            }
        }
        None
    }
}

impl AiEngine for LlmAi {
    /// 获取 AI 最佳走法。
    ///
    /// TODO: 当前使用阻塞 HTTP 客户端 (`reqwest::blocking`)，
    /// 在 GUI 线程调用会冻结界面。上层应在独立线程
    /// (`std::thread::spawn` 或 `tauri::async_runtime::spawn_blocking`) 中调用此方法，
    /// 或改用 async 版本。
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        let prompt = Self::board_to_prompt(board, color);
        let client = reqwest::blocking::Client::new();
        let body = serde_json::json!({
            "model": self.model,
            "messages": [
                {"role": "user", "content": prompt}
            ],
            "max_tokens": 50,
            "temperature": 0.3
        });

        let resp = client
            .post(&self.endpoint)
            .header("Authorization", format!("Bearer {}", self.api_key))
            .header("Content-Type", "application/json")
            .json(&body)
            .send()
            .ok()?;

        let json: serde_json::Value = resp.json().ok()?;
        let content = json["choices"][0]["message"]["content"].as_str()?;
        Self::parse_response(content)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_coordinate() {
        assert_eq!(LlmAi::parse_response("7,8"), Some(Position::new(7, 8)));
        assert_eq!(LlmAi::parse_response("(7, 8)"), Some(Position::new(7, 8)));
        assert_eq!(
            LlmAi::parse_response("坐标是 10,5"),
            Some(Position::new(10, 5))
        );
        assert_eq!(LlmAi::parse_response("no coordinate"), None);
    }

    #[test]
    fn test_board_to_prompt() {
        let board = Board::new(15);
        let prompt = LlmAi::board_to_prompt(&board, Color::Black);
        assert!(prompt.contains("黑棋(1)"));
        assert!(prompt.contains("0 0 0"));
    }
}
