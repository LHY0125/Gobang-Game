# LLM AI 四项改进 — 设计文档

> 状态: 待审核

## 目标

提升 LLM AI 的可用性和体验：前端可配置化、提示词专业化、异步化、流式输出。

## 架构概览

四项改进相互关联但不耦合，可独立实施：

```
┌─────────────────────────────────────────────────────────────┐
│  前端 (React)                                                │
│  ┌──────────────┐  ┌──────────────────────────────────────┐ │
│  │ AiGameSetup  │  │ GameView                             │ │
│  │ + AI 类型选择 │  │ + LLM 思考过程显示（流式文本渲染）     │ │
│  │ + LLM 配置   │  │ + 监听 llm-stream / llm-move 事件     │ │
│  └──────┬───────┘  └────────────────┬─────────────────────┘ │
│         │ config.useLlm             │ invoke('ai_move_llm') │
│         │ config.llmEndpoint         │ listen('llm-stream')  │
│         │ config.llmApiKey           │ listen('llm-move')    │
│         │ config.llmModel            │                       │
├─────────┼───────────────────────────┼───────────────────────┤
│  Rust   │                           │                       │
│  ┌──────┴───────────────────────────┴──────────────────────┐│
│  │ Tauri commands.rs                                        ││
│  │                                                          ││
│  │ ai_move()        → AlphaBetaAi (同步,线程池) 保持不变    ││
│  │ ai_move_llm()    → LlmAi (异步,SSE 流式)  新增          ││
│  └──────────────────────────┬───────────────────────────────┘│
│  ┌──────────────────────────┴───────────────────────────────┐│
│  │ core/llm.rs                                               ││
│  │                                                          ││
│  │ LlmAi {                                                  ││
│  │   client: reqwest::Client,  ← 异步,复用一个实例          ││
│  │   endpoint, api_key, model                               ││
│  │ }                                                        ││
│  │                                                          ││
│  │ board_to_prompt()  → 新的结构化提示词                    ││
│  │ parse_response()   → unchanged                           ││
│  │ stream_move()      → 新增: 流式请求+SSE解析              ││
│  └──────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

---

## P1: 前端 LLM 配置 UI

### 现状

`AiGameSetup.tsx` 仅有难度、先手、禁手三个选项。`GameConfig` 已有 `useLlm`/`llmEndpoint`/`llmApiKey`/`llmModel` 字段但前端从未暴露。

### 方案

在 `AiGameSetup.tsx` 中增加 AI 类型切换和 LLM 配置区域：

```
┌──────────────────────────────┐
│ 人机对战                      │
│                              │
│ AI 类型:  [Alpha-Beta ▼]     │  ← 新增：下拉切换
│ 棋盘大小: [15×15 ▼]          │
│                              │
│ -- 以下仅在 Alpha-Beta 时显示 │
│ AI 难度:  [3 ▼]              │
│                              │
│ -- 以下仅在 LLM 时显示        │
│ API 地址: [_______________]  │  ← 新增
│ API Key:  [_______________]  │  ← 新增 (type=password)
│ 模型:     [_______________]  │  ← 新增
│                              │
│ 先手: [黑棋 ▼]               │
│ ☑ 禁手规则                    │
│                              │
│ [新游戏]  [返回]              │
└──────────────────────────────┘
```

### 实现细节

**状态管理:**
```typescript
const [aiType, setAiType] = useState<'alpha-beta' | 'llm'>('alpha-beta');
const [llmEndpoint, setLlmEndpoint] = useState('https://api.openai.com/v1/chat/completions');
const [llmApiKey, setLlmApiKey] = useState('');
const [llmModel, setLlmModel] = useState('gpt-4o-mini');
```

**handleStart:**
```typescript
const handleStart = async () => {
  const config: GameConfig = {
    // ... existing fields ...
    useLlm: aiType === 'llm',
    llmEndpoint,
    llmApiKey,
    llmModel,
  };
  await startGame('VsAi', config);
  onStart();
};
```

**i18n 新增键:**

| 键 | 中文 | English |
|----|------|---------|
| `ai_setup.ai_type` | AI 类型 | AI Type |
| `ai_setup.alpha_beta` | Alpha-Beta 搜索 | Alpha-Beta Search |
| `ai_setup.llm` | 大语言模型 | Large Language Model |
| `ai_setup.llm_endpoint` | API 地址 | API Endpoint |
| `ai_setup.llm_api_key` | API 密钥 | API Key |
| `ai_setup.llm_model` | 模型 | Model |
| `ai_setup.llm_endpoint_placeholder` | 输入 API 地址... | Enter API endpoint... |
| `ai_setup.llm_model_placeholder` | 例如 gpt-4o-mini | e.g. gpt-4o-mini |

### 影响范围

| 文件 | 操作 |
|------|------|
| `src/components/menu/AiGameSetup.tsx` | 修改：增加 AI 类型切换 + LLM 表单 |
| `src/i18n/zh-CN.json` | 修改：新增 8 个键 |
| `src/i18n/en.json` | 修改：新增 8 个键 |

---

## P2: 提示词重写

### 现状

当前提示词仅包含数字矩阵 + 一句话："你是黑棋(1), 请返回最佳落子坐标 (格式: x,y)"。LLM 缺乏上下文指导，容易返回不合法坐标或随意走棋。

### 方案

采用 OpenAI Chat Completions 的 `messages` 数组格式，使用 system + user 双消息结构：

**System message（角色设定）:**
```
你是一位世界级五子棋(Gomoku)AI，精通开局定式、中盘攻防与残局计算。
你严格遵循五子棋规则，在15×15棋盘上对弈。
```

**User message（结构化棋盘分析请求）:**

```
## 规则
- 黑棋先手，五子连珠获胜（横/竖/斜均可）
- 黑棋禁手：禁止双三、双四、长连（六子及以上）
- 棋盘大小：15×15，坐标范围 0-14

## 当前棋盘
（数字矩阵：0=空 1=黑 2=白）

   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
0  .................................（具体棋子）
...

最后一手: 黑棋 (7,7)

## 对局分析要求
请按以下步骤分析：
1. 评估当前局面（开局/中盘/残局阶段）
2. 识别双方所有活三、活四、冲四等关键棋形
3. 判断是否有必须立即防守的威胁
4. 列出 2-3 个候选落子点，比较优劣
5. 选择最佳落子

## 输出格式
分析:
（你的详细分析）

坐标: x,y
```

### 关键改进

| 维度 | 旧 | 新 |
|------|----|----|
| 角色 | 无 | system message 设定 AI 身份 |
| 规则 | 无 | 明确禁手规则、棋盘范围 |
| 棋盘可读性 | 纯数字矩阵 | 带坐标行列标号 |
| 分析指导 | 无 | 5 步分析流程，引导 LLM 推理 |
| 输出解析 | 直接扫描 "x,y" | 结构化输出（分析段 + 坐标行），更易解析 |
| 最后一手 | 无 | 标注最后一手位置，帮助 LLM 定位热点区域 |

### 坐标解析

`parse_response()` 保持不变 — 它扫描文本中第一个 `数字,数字` 模式。新的输出格式中 `坐标: x,y` 行会被正确捕获。同时也能兼容 LLM 输出变体（如 `最佳坐标: (7,8)` 或直接 `7,8`）。

### 影响范围

| 文件 | 操作 |
|------|------|
| `core/src/llm.rs` | 重写 `board_to_prompt()` → 新函数 `build_messages()` |

---

## P3: Async reqwest 迁移

### 现状

- `Cargo.toml`: `reqwest = { features = ["json", "blocking"] }`
- `LlmAi::best_move()`: 每次调用 `reqwest::blocking::Client::new()`，创建临时客户端
- `commands::ai_move()`: 用 `std::thread::spawn` 包装以避免阻塞 GUI

### 问题

1. `reqwest::blocking` 在独立线程中运行，无法利用 Tauri 的 tokio 异步运行时
2. 每次调用新建 Client，不复用连接池
3. 阻塞模式不支持 SSE streaming（P4 的前置依赖）

### 方案

**不在 `AiEngine` trait 层面做异步改造** — `best_move()` 保持同步，继续服务于 AlphaBetaAi。为 LLM AI 新增独立的异步命令路径。

**Cargo.toml 变化：**
```toml
# core/Cargo.toml
reqwest = { version = "0.12", features = ["json", "stream"] }  # 去 blocking, 加 stream
```

**LlmAi 结构变化：**
```rust
pub struct LlmAi {
    client: reqwest::Client,  // 新增：异步客户端，复用连接池
    endpoint: String,
    api_key: String,
    model: String,
}

impl LlmAi {
    pub fn new(endpoint: &str, api_key: &str, model: &str) -> Self {
        Self {
            client: reqwest::Client::new(),  // 一次创建，全局复用
            endpoint: endpoint.to_string(),
            api_key: api_key.to_string(),
            model: model.to_string(),
        }
    }
}
```

**`AiEngine` trait 的 `best_move()` 保持同步不变** — 当 LlmAi 的 `best_move()` 被调用时（如果将来有场景需要），内部用 `tokio::runtime::Handle::current().block_on()` 桥接。但实际上有了 `ai_move_llm` 后，`best_move()` 对 LlmAi 的使用场景消失。

### 新增 Tauri 命令

```rust
// gui/src/commands.rs
#[tauri::command]
async fn ai_move_llm(state: State<'_, AppState>, app: tauri::AppHandle) 
    -> Result<Option<(usize, usize)>, String> 
{
    // 读取 board、color、LLM 配置（不走 ai_engine trait）
    let (board, color, endpoint, api_key, model) = { /* 锁内提取 */ };
    
    // 构造 LlmAi
    let llm = LlmAi::new(&endpoint, &api_key, &model);
    
    // 异步流式调用（见 P4）
    llm.stream_move(&board, color, &app).await
}
```

### 影响范围

| 文件 | 操作 |
|------|------|
| `core/Cargo.toml` | reqwest features: `blocking` → `stream` |
| `core/src/llm.rs` | `LlmAi` 增加 `client: reqwest::Client` 字段；新增 `stream_move()` 方法 |
| `gui/Cargo.toml` | 可能需要增加 `tokio` 依赖（Tauri 自带） |
| `gui/src/commands.rs` | 新增 `ai_move_llm` 命令 |
| `gui/src/lib.rs` | 注册 `ai_move_llm` 命令 |

### 向后兼容

- `ai_move` 命令保持不变，继续服务于 AlphaBetaAi
- `AiEngine` trait 不变
- `GameConfig` 不变
- 仅当 `config.use_llm == true` 时前端走新命令路径

---

## P4: SSE 流式输出

### 现状

LLM AI 调用期间，前端显示静态文字 "AI 思考中..."。用户无法判断是否卡死、LLM 正在分析什么、预计还需多久。

### 方案

通过 OpenAI 的 `stream: true` 参数启用 SSE，逐 token 推送思考过程到前端。

**数据流：**

```
LLM API (OpenAI 兼容)              Rust Backend                React Frontend
─────────────────                  ─────────────                ──────────────
                                   
POST /chat/completions             ai_move_llm()                gameStore.aiMove()
{stream:true}                      │                            │
  │                                │                            │
  ├─ data: {"choices":[{"delta":   │                            │
  │    {"content":"分析"}}]}  ───► 解析 content                  │
  │                                │                            │
  │                                app.emit("llm-stream",       │
  │                                  "分析")  ─────────────────► 追加到思考文本
  │                                                             │
  ├─ data: {"choices":[{"delta":   │                            │
  │    {"content":"：\n"}}]}  ───► 解析 content                  │
  │                                │                            │
  │                                app.emit("llm-stream",       │
  │                                  "：\n")  ─────────────────► 追加到思考文本
  │                                                             │
  ├─ ...更多 token...               ...更多事件...               ...实时渲染...
  │                                                             │
  └─ data: {"choices":[{"finish_   │                            │
       reason":"stop"}]}           │                            │
                                   │                            │
                                   最终解析完整响应文本            │
                                   parse_response(full_content)  │
                                   │                            │
                                   app.emit("llm-move",         │
                                     {x, y})  ─────────────────► 自动落子
```

### Rust 实现

```rust
impl LlmAi {
    /// 流式调用 LLM，通过 Tauri 事件推送思考过程
    pub async fn stream_move(
        &self,
        board: &Board,
        color: Color,
        app: &tauri::AppHandle,
    ) -> Result<Option<(usize, usize)>, String> {
        let messages = Self::build_messages(board, color);
        
        let body = serde_json::json!({
            "model": self.model,
            "messages": messages,
            "max_tokens": 512,
            "temperature": 0.3,
            "stream": true
        });

        let resp = self.client
            .post(&self.endpoint)
            .header("Authorization", format!("Bearer {}", self.api_key))
            .header("Content-Type", "application/json")
            .json(&body)
            .send()
            .await
            .map_err(|e| format!("LLM 请求失败: {}", e))?;

        let mut full_content = String::new();
        let mut stream = resp.bytes_stream();
        
        use futures_util::StreamExt;
        while let Some(chunk) = stream.next().await {
            let chunk = chunk.map_err(|e| format!("流读取失败: {}", e))?;
            let text = String::from_utf8_lossy(&chunk);
            
            // SSE 格式: "data: {...}\n\n"
            for line in text.lines() {
                let line = line.trim();
                if line.is_empty() || !line.starts_with("data: ") {
                    continue;
                }
                let json_str = &line[6..]; // 去掉 "data: " 前缀
                if json_str == "[DONE]" {
                    break;
                }
                if let Ok(parsed) = serde_json::from_str::<serde_json::Value>(json_str) {
                    if let Some(content) = parsed["choices"][0]["delta"]["content"].as_str() {
                        full_content.push_str(content);
                        let _ = app.emit("llm-stream", content);
                    }
                }
            }
        }

        // 解析最终坐标
        let pos = Self::parse_response(&full_content);
        if let Some(p) = pos {
            let _ = app.emit("llm-move", serde_json::json!({"x": p.x, "y": p.y}));
        }
        Ok(pos.map(|p| (p.x, p.y)))
    }
}
```

### 前端实现

**gameStore 修改:**
```typescript
// 新增：LLM 流式思考状态
llmThinking: string;           // 当前累积的思考文本
setLlmThinking: (text: string) => void;
appendLlmThinking: (chunk: string) => void;

aiMoveLlm: async () => {
  set({ status: 'ai_thinking', llmThinking: '' });
  const pos: [number, number] | null = await invoke('ai_move_llm');
  if (pos) {
    await get().placePiece(pos[0], pos[1]);
    if (get().status !== 'game_over') {
      set({ status: 'playing' });
    }
  }
}
```

**GameView 修改:**
```tsx
// 监听流式事件
useEffect(() => {
  const setup = async () => {
    unlisten1 = await listen<string>('llm-stream', (e) => {
      appendLlmThinking(e.payload);
    });
    unlisten2 = await listen<{x: number, y: number}>('llm-move', (e) => {
      // move event handled by the store
    });
  };
  setup();
  return () => { unlisten1?.(); unlisten2?.(); };
}, []);

// 当 useLlm 时，显示思考过程而非静态文字
{status === 'ai_thinking' && (
  <div className="llm-thinking">
    <div className="llm-thinking-header">AI 思考中...</div>
    <pre className="llm-thinking-text">{llmThinking}</pre>
  </div>
)}
```

### 前端调用分流

```typescript
// gameStore.aiMove() 修改为：
aiMove: async () => {
  const { config } = get();
  if (config.useLlm) {
    await get().aiMoveLlm();
  } else {
    // 原有逻辑：invoke('ai_move') → placePiece
    set({ status: 'ai_thinking' });
    const pos = await invoke('ai_move');
    // ...
  }
}
```

### 新增依赖

- `futures-util` (core/Cargo.toml) — `StreamExt` for bytes_stream

### 影响范围

| 文件 | 操作 |
|------|------|
| `core/Cargo.toml` | 新增 `futures-util` 依赖 |
| `core/src/llm.rs` | 新增 `build_messages()`, `stream_move()` 方法；`best_move()` 保持兼容 |
| `gui/src/commands.rs` | 新增 `ai_move_llm` async 命令 |
| `gui/src/lib.rs` | 注册新命令 |
| `src/store/gameStore.ts` | 新增 `llmThinking`, `appendLlmThinking`, `aiMoveLlm`；修改 `aiMove` |
| `src/components/game/GameView.tsx` | 监听 `llm-stream`/`llm-move` 事件，渲染思考文本 |
| `src/i18n/zh-CN.json` | 新增 `game.llm_thinking` |
| `src/i18n/en.json` | 新增 `game.llm_thinking` |

---

## 文件变更汇总

| 文件 | P1 | P2 | P3 | P4 | 总变更 |
|------|:--:|:--:|:--:|:--:|:------:|
| `core/Cargo.toml` | | | ● | ● | 修改 |
| `core/src/llm.rs` | | ● | ● | ● | 重写 |
| `gui/src/commands.rs` | | | ● | ● | 新增命令 |
| `gui/src/lib.rs` | | | ● | ● | 注册命令 |
| `src/components/menu/AiGameSetup.tsx` | ● | | | | 修改 |
| `src/store/gameStore.ts` | | | | ● | 修改 |
| `src/components/game/GameView.tsx` | | | | ● | 修改 |
| `src/i18n/zh-CN.json` | ● | | | ● | 修改 |
| `src/i18n/en.json` | ● | | | ● | 修改 |

---

## 测试计划

### P1 (前端 UI)
- [ ] 渲染测试：切换 AI 类型时正确显示/隐藏对应配置区域
- [ ] handleStart 传递正确的 useLlm 和 LLM 字段到 GameConfig
- [ ] API Key 输入框 type="password" 不泄露密钥

### P2 (提示词)
- [ ] `build_messages()` 返回正确的 system + user 两个消息
- [ ] 提示词包含规则说明、棋盘矩阵、输出格式要求
- [ ] `parse_response()` 能从新格式中正确提取坐标

### P3 (异步迁移)
- [ ] `LlmAi::new()` 创建的 Client 可复用
- [ ] `ai_move_llm` 能成功完成 HTTP 请求
- [ ] AlphaBetaAi 的 `ai_move` 路径不受影响

### P4 (流式输出)
- [ ] SSE 事件正确解析每个 token
- [ ] `llm-stream` 事件按序推送到前端
- [ ] `llm-move` 在流结束后触发，携带正确坐标
- [ ] 前端思考文本实时累积显示
- [ ] 流结束后思考文本自动清除（下次 AI 思考时）

---

## 顺序与依赖

```
P1 (前端 UI) ──→ 可独立实施，无需等待
P2 (提示词)  ──→ 可独立实施，无需等待
P3 (异步)    ──→ 可独立实施，但 P4 依赖 P3
P4 (SSE流式) ──→ 依赖 P3（需要 stream feature + async client）

推荐顺序: P1 → P3 → P4 → P2
（P1 最先因为用户能立即看到效果；P3→P4 是技术链；P2 最后微调体验）
```
