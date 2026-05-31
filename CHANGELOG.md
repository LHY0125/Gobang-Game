# Changelog

## 2.0.1 (2026-05-31)

### Fixed
- 修复悔棋奇数步崩溃（新增 NoHistory 错误语义）
- AI 搜索使用 Arc + 独立线程执行，不阻塞 GUI
- 计时器改为双方独立象棋钟，归零自动判负
- 补全全部 i18n 硬编码中文，中英双语完整
- 棋谱日期从 Unix 时间戳改为 ISO 8601 格式
- 禁用未完成的网络对战入口，避免用户困惑
- clippy needless_range_loop 警告

### Added
- 前端核心逻辑和棋盘渲染 10 个 vitest 单元测试
- LLM AI 接入 GUI（GameConfig.useLlm 切换）
- 认输和保存棋谱 JSON 下载功能
- 棋盘大小选择器（9x9 ~ 19x19）
- React Error Boundary 组件防白屏
- env_logger 基础日志系统
- CONTRIBUTING.md 修正

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
