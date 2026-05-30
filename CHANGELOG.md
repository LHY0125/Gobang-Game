# Changelog

## 2.0.0 (2026-05-31)

### Added
- Rust + Tauri 2.x + React 19 + TypeScript strict 全重写
- Cargo workspace 两 crate 架构 (core + gui)
- Canvas 木纹风格棋盘渲染
- 中英双语界面 (i18next)
- Alpha-Beta 剪枝 AI 引擎 (5 级难度)
- LLM 大模型 AI (OpenAI 兼容 API)
- renet 网络对战 (纯 Rust ENet 协议)
- JSON 棋谱记录与回放
- Zustand 状态管理

### Changed
- 从 C + IUP + CMake 迁移到 Rust + Tauri + Vite
- 棋谱格式从二进制改为 JSON
- 网络协议从 ENet C 库改为 renet 纯 Rust 实现
- AI 从评分制升级为 Alpha-Beta 搜索
