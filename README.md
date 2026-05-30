# Gobang (五子棋) v2.0

Rust + Tauri 2.x + React 19 构建的五子棋桌面应用。

## 功能

- 本地双人对战
- 人机对战 (Alpha-Beta 剪枝 AI, 5 级难度)
- 网络对战 (renet P2P)
- LLM 大模型 AI
- 棋谱记录与回放 (JSON)
- 禁手规则 (长连/双三/双四)
- 中/英双语

## 开发

### 环境要求

- Node.js 22+
- Rust 1.95+ (stable-x86_64-pc-windows-gnu)
- MinGW-w64
- Windows 10+

### 命令

```bash
npm install         # 安装前端依赖
npx tauri dev       # 开发模式
npx tauri build     # 生产构建
cargo test          # Rust 测试
cargo clippy -- -D warnings  # Lint
npm test            # 前端测试
```

## 架构

```
core/   # Rust 游戏核心库 (零 Tauri 依赖)
gui/    # Tauri 桌面应用 (薄命令层)
src/    # React 前端 (TypeScript strict)
```

## 许可

MIT
