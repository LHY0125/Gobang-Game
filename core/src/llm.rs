use crate::ai::AiEngine;
use crate::board::Board;
use crate::types::{CellState, Color, Position};
use futures_util::StreamExt;
use serde_json::Value;

/// 大模型 AI — 通过 HTTP API 调用（异步 + SSE 流式）
pub struct LlmAi {
    client: reqwest::Client,
    endpoint: String,
    api_key: String,
    model: String,
}

impl LlmAi {
    pub fn new(endpoint: &str, api_key: &str, model: &str) -> Self {
        Self {
            client: reqwest::Client::new(),
            endpoint: endpoint.to_string(),
            api_key: api_key.to_string(),
            model: model.to_string(),
        }
    }

    /// 构造结构化提示词（system + user 双消息），供 gui 层和 AiEngine 复用
    pub fn build_messages(board: &Board, color: Color) -> Vec<Value> {
        let color_str = match color {
            Color::Black => "黑棋(1)",
            Color::White => "白棋(2)",
        };
        let opponent_str = match color {
            Color::Black => "白棋(2)",
            Color::White => "黑棋(1)",
        };

        let system = format!(
            "你是一位世界级五子棋(Gomoku)AI，精通开局定式、中盘攻防与残局计算。\
             你严格遵循五子棋规则，在{size}×{size}棋盘上对弈。\
             你当前执{color}，对手执{opponent}。",
            size = board.size,
            color = color_str,
            opponent = opponent_str
        );

        let user = build_user_prompt(board, color);

        vec![
            serde_json::json!({"role": "system", "content": system}),
            serde_json::json!({"role": "user", "content": user}),
        ]
    }

    /// 流式调用 LLM，返回累积的完整响应文本。
    /// 每收到一个 token 时调用 `on_token` 回调，供 gui 层发射 Tauri 事件。
    pub async fn stream_move(
        &self,
        board: &Board,
        color: Color,
        on_token: &(impl Fn(&str) + Send + Sync),
    ) -> Result<String, String> {
        let messages = Self::build_messages(board, color);

        let body = serde_json::json!({
            "model": self.model,
            "messages": messages,
            "max_tokens": 512,
            "temperature": 0.3,
            "stream": true
        });

        let resp = self
            .client
            .post(&self.endpoint)
            .header("Authorization", format!("Bearer {}", self.api_key))
            .header("Content-Type", "application/json")
            .json(&body)
            .send()
            .await
            .map_err(|e| format!("LLM 请求失败: {}", e))?;

        if !resp.status().is_success() {
            let status = resp.status();
            let text = resp.text().await.unwrap_or_default();
            return Err(format!("LLM API 返回错误 ({}): {}", status, text));
        }

        let mut full_content = String::new();
        let mut stream = resp.bytes_stream();

        while let Some(chunk_result) = stream.next().await {
            let chunk = chunk_result.map_err(|e| format!("流读取失败: {}", e))?;
            let text = String::from_utf8_lossy(&chunk);

            for line in text.lines() {
                let line = line.trim();
                if line.is_empty() || !line.starts_with("data:") {
                    continue;
                }
                let json_str = line.strip_prefix("data:").unwrap().trim();
                if json_str == "[DONE]" {
                    break;
                }
                if let Ok(parsed) = serde_json::from_str::<Value>(json_str) {
                    if let Some(content) = parsed["choices"][0]["delta"]["content"].as_str() {
                        full_content.push_str(content);
                        on_token(content);
                    }
                }
            }
        }

        if full_content.is_empty() {
            return Err("LLM 未返回任何内容".to_string());
        }
        Ok(full_content)
    }

    /// 解析 LLM 响应中的坐标。
    /// 先用正则匹配 `数字,数字` 模式（支持中英文逗号、括号），
    /// 失败则回退到逐字节扫描。
    pub fn parse_response(response: &str) -> Option<Position> {
        // 正则优先：匹配 (12,34) / 12,34 / 12，34 等格式
        let re = regex::Regex::new(r"\(?\s*(\d{1,2})\s*[,，]\s*(\d{1,2})\s*\)?").ok()?;
        if let Some(caps) = re.captures(response) {
            let x = caps.get(1)?.as_str().parse::<usize>().ok()?;
            let y = caps.get(2)?.as_str().parse::<usize>().ok()?;
            return Some(Position::new(x, y));
        }
        // 回退：逐字节扫描
        parse_fallback(response)
    }
}

/// 逐字节扫描坐标（正则匹配失败时的回退方案）
fn parse_fallback(response: &str) -> Option<Position> {
    let bytes = response.as_bytes();
    for (i, &b) in bytes.iter().enumerate() {
        if b != b',' {
            continue;
        }
        let x_start = (0..i)
            .rev()
            .take_while(|&j| bytes[j].is_ascii_digit())
            .last()
            .unwrap_or(i);
        if x_start == i {
            continue;
        }
        let y_start = (i + 1..bytes.len()).find(|&j| bytes[j].is_ascii_digit())?;
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

impl AiEngine for LlmAi {
    /// 同步接口 — 通过当前 tokio runtime 桥接异步调用（不含流式输出）。
    /// 前端应优先使用 `ai_move_llm` 命令以获得流式体验。
    fn best_move(&self, board: &Board, color: Color) -> Option<Position> {
        let messages = Self::build_messages(board, color);

        let body = serde_json::json!({
            "model": self.model,
            "messages": messages,
            "max_tokens": 200,
            "temperature": 0.3,
            "stream": false
        });

        let result: Result<_, String> = tokio::runtime::Handle::try_current()
            .map_err(|_| "无 tokio runtime".to_string())
            .and_then(|handle| {
                handle.block_on(async {
                    let resp = self
                        .client
                        .post(&self.endpoint)
                        .header("Authorization", format!("Bearer {}", self.api_key))
                        .header("Content-Type", "application/json")
                        .json(&body)
                        .send()
                        .await
                        .map_err(|e| format!("请求失败: {}", e))?;

                    let json: Value = resp.json().await.map_err(|e| format!("解析失败: {}", e))?;

                    let content = json["choices"][0]["message"]["content"]
                        .as_str()
                        .ok_or("LLM 响应为空")?;
                    Ok(Self::parse_response(content))
                })
            });

        match result {
            Ok(pos) => pos,
            Err(e) => {
                log::error!("LLM 同步调用失败: {}", e);
                None
            }
        }
    }
}

/// 构造用户提示词（棋盘矩阵 + 分析指导）
fn build_user_prompt(board: &Board, color: Color) -> String {
    let mut s = String::new();

    // 规则说明
    s.push_str("## 规则\n");
    s.push_str("- 黑棋先手，五子连珠获胜（横/竖/斜均可）\n");
    s.push_str("- 黑棋禁手：禁止双三、双四、长连（六子及以上）\n");
    s.push_str(&format!(
        "- 棋盘大小：{size}×{size}，坐标范围 0-{max}\n",
        size = board.size,
        max = board.size - 1
    ));
    s.push('\n');

    // 棋盘矩阵（带行列标号）
    s.push_str("## 当前棋盘\n");
    s.push_str("（0=空 1=黑 2=白）\n\n");
    s.push_str("   ");
    for y in 0..board.size {
        s.push_str(&format!("{:>2}", y % 10));
    }
    s.push('\n');
    for x in 0..board.size {
        s.push_str(&format!("{:>2} ", x));
        for y in 0..board.size {
            let ch = match board.get(Position::new(x, y)) {
                CellState::Empty => ".",
                CellState::Occupied(Color::Black) => "1",
                CellState::Occupied(Color::White) => "2",
            };
            s.push_str(&format!("{:>2}", ch));
        }
        s.push('\n');
    }
    s.push('\n');

    // 最后一手标注
    if let Some(last_move) = board.history().last() {
        let color_name = match last_move.color {
            Color::Black => "黑棋",
            Color::White => "白棋",
        };
        s.push_str(&format!(
            "最后一手: {} ({},{})\n\n",
            color_name, last_move.position.x, last_move.position.y
        ));
    }

    // 分析指导
    let color_str = match color {
        Color::Black => "黑棋(1)",
        Color::White => "白棋(2)",
    };
    s.push_str("## 对局分析要求\n");
    s.push_str("请按以下步骤分析：\n");
    s.push_str("1. 评估当前局面（开局/中盘/残局阶段）\n");
    s.push_str("2. 识别双方所有活三、活四、冲四等关键棋形\n");
    s.push_str("3. 判断是否有必须立即防守的威胁\n");
    s.push_str("4. 列出 2-3 个候选落子点，比较优劣\n");
    s.push_str("5. 选择最佳落子\n\n");

    s.push_str(&format!("你是{}, 请分析并走棋。\n\n", color_str));
    s.push_str("## 输出格式\n");
    s.push_str("分析:\n（详细分析）\n\n坐标: x,y\n");

    s
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
        assert_eq!(
            LlmAi::parse_response("坐标: 14,0"),
            Some(Position::new(14, 0))
        );
        assert_eq!(LlmAi::parse_response("no coordinate"), None);
        assert_eq!(LlmAi::parse_response(""), None);
    }

    #[test]
    fn test_parse_coordinate_with_analysis_prefix() {
        let response = "分析:\n黑棋有活三优势\n\n坐标: 7,8";
        assert_eq!(LlmAi::parse_response(response), Some(Position::new(7, 8)));
    }

    #[test]
    fn test_build_messages_structure() {
        let board = Board::new(15);
        let messages = LlmAi::build_messages(&board, Color::Black);
        assert_eq!(messages.len(), 2);
        assert_eq!(messages[0]["role"], "system");
        assert_eq!(messages[1]["role"], "user");

        let user_content = messages[1]["content"].as_str().unwrap();
        assert!(user_content.contains("## 规则"));
        assert!(user_content.contains("## 当前棋盘"));
        assert!(user_content.contains("## 对局分析要求"));
        assert!(user_content.contains("## 输出格式"));
        assert!(user_content.contains("坐标:"));
    }

    #[test]
    fn test_build_messages_includes_board() {
        let mut board = Board::new(15);
        board = board.place(Position::new(7, 7), Color::Black).unwrap();
        board = board.place(Position::new(7, 8), Color::White).unwrap();

        let messages = LlmAi::build_messages(&board, Color::Black);
        let user_content = messages[1]["content"].as_str().unwrap();

        assert!(user_content.contains("."));
        assert!(user_content.contains("1"));
        assert!(user_content.contains("2"));
        assert!(user_content.contains("最后一手"));
        assert!(user_content.contains("7,8"));
    }
}
