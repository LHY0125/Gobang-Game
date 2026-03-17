# C语言五子棋系统

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Version](https://img.shields.io/badge/version-v9.0-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

> 🎯 **最新版本 v9.0** - 全面集成 ENet 网络联机功能，重构 GUI 代码架构，纯图形化交互体验。

## 📋 大版本更新

### v9.0 (2026-03-17) - 局域网联机与 GUI 架构重构

- 🌐 **局域网对战** - 引入 ENet 库，支持稳定、无粘包的局域网多人对战（创建房间/加入房间）。
- 🎨 **GUI 深度重构** - 拆分臃肿的 GUI 代码，模块化为 `gui_core`, `gui_game`, `gui_menu`, `gui_draw`, `gui_replay`，大幅提升可维护性。
- 📦 **精简安装包** - 优化 Inno Setup 打包脚本，实现极简核心依赖打包。
- � **全面摒弃控制台** - 专注于基于 IUP 的原生图形化交互体验。

---

## 目录

- [C语言五子棋系统](#c语言五子棋系统)
  - [📋 大版本更新](#-大版本更新)
    - [v9.0 (2026-03-17) - 局域网联机与 GUI 架构重构](#v90-2026-03-17---局域网联机与-gui-架构重构)
  - [目录](#目录)
  - [项目简介](#项目简介)
  - [功能特性](#功能特性)
    - [🎮 游戏模式](#-游戏模式)
    - [⚙️ 核心功能](#️-核心功能)
  - [快速开始](#快速开始)
    - [编译项目](#编译项目)
    - [打包安装程序](#打包安装程序)
  - [游戏玩法](#游戏玩法)
  - [技术架构](#技术架构)
    - [AI 算法](#ai-算法)
    - [网络通信](#网络通信)
    - [图形界面](#图形界面)
  - [项目结构](#项目结构)
  - [许可证](#许可证)

---

## 项目简介

这是一个使用纯 C 语言编写的现代化五子棋系统。采用轻量级的 IUP 库构建原生风格的图形化用户界面。系统不仅支持基于 Minimax 算法（带 Alpha-Beta 剪枝）的高级人机对战，还完美支持基于 ENet 的局域网多人实时联机对弈。

## 功能特性

### 🎮 游戏模式

- **人机对战 (PvE)** - 挑战 1-5 级难度的智能 AI。
- **局域网联机 (Network)** - 建立主机或作为客机加入，与朋友在局域网内实时对战。
- **复盘模式 (Replay)** - 载入自动保存的对局记录（CSV），逐步回放并分析对局。

### ⚙️ 核心功能

- **实时对局控制** - 支持悔棋（PvE）、断线重连检测、胜负判定。
- **自动游戏记录** - 每局游戏结束后自动保存至 `bin/records/` 目录。
- **模块化构建** - 提供高度配置化的 Makefile，一键源码编译包含 ENet 和主程序的二进制文件。

---

## 快速开始

### 编译项目

项目使用 Makefile 进行构建，依赖 MinGW-w64 工具链。
在项目根目录下打开 PowerShell 或 CMD，运行：

```bash
# 清理旧文件
mingw32-make clean

# 编译生成 GUI 可执行文件
mingw32-make gui
```

生成的可执行文件 `gobang_gui.exe` 和所需的动态链接库 `iup.dll` 会输出到 `bin/` 目录下。

**运行游戏：**

```bash
.\bin\gobang_gui.exe
```

### 打包安装程序

如果你需要将游戏打包成 Windows 安装程序，项目提供了 Inno Setup 脚本。
确保你已安装 [Inno Setup 6](https://jrsoftware.org/isinfo.php)，然后运行：

```powershell
& "D:\Program Files (x86)\Inno Setup 6\iscc.exe" installer\installer.iss
```

生成的安装包 `Gobang_Inno_Setup.exe` 将存放在 `installer\dist\` 目录下。

---

## 游戏玩法

1. 运行 `gobang_gui.exe` 启动游戏主菜单。
2. 选择你要进行的游戏模式：
   - **人机对战**：直接进入游戏，玩家执黑先行，点击棋盘空白处落子。
   - **局域网联机**：
     - **作为房主**：点击“局域网联机” -> “创建房间”，等待好友加入（默认端口8888）。
     - **作为玩家**：点击“局域网联机” -> 输入房主的 IP 地址和端口 -> “加入房间”。
   - **复盘模式**：选择历史对局记录，使用“上一步/下一步”按钮进行回放。

---

## 技术架构

### AI 算法

- **Minimax 博弈树**：遍历所有可能的落子情况。
- **Alpha-Beta 剪枝**：极大提升搜索效率，AI 默认思考深度为 3 层。
- **启发式评估函数**：基于连五、活四、冲四、活三等经典五子棋棋型进行棋局评分。

### 网络通信

- **底层库**：[ENet](http://enet.bespin.org/) (基于 UDP 的可靠网络传输协议)。
- **机制**：游戏采用非阻塞轮询机制处理网络事件，完美解决传统 TCP 粘包问题，确保落子坐标、断开连接等指令 100% 可靠送达。

### 图形界面

- **底层库**：[IUP](https://www.tecgraf.puc-rio.br/iup/) (轻量级、跨平台的 C 语言原生 GUI 库)。
- **机制**：事件驱动架构，通过回调函数响应用户的鼠标点击、按钮触发和窗口切换。

---

## 项目结构

```text
Gobang/
├── bin/                    # 编译输出目录 (包含 exe, dll 和 records)
├── include/                # 头文件目录
├── libs/                   # 第三方库源码及预编译库 (ENet, IUP)
├── src/                    # 核心源代码
│   ├── main.c              # 程序入口
│   ├── gobang.c            # 五子棋核心规则与逻辑
│   ├── ai.c                # 智能 AI 算法
│   ├── network.c           # ENet 网络通信封装
│   ├── record.c            # 棋谱记录与解析
│   ├── gui_core.c          # GUI 初始化与核心循环
│   ├── gui_game.c          # 游戏主界面与交互逻辑
│   ├── gui_menu.c          # 游戏主菜单界面
│   ├── gui_draw.c          # 棋盘与棋子渲染
│   └── gui_replay.c        # 复盘模式交互逻辑
├── installer/              # Inno Setup 安装包打包脚本及输出
├── Makefile                # 自动化构建脚本
└── README.md               # 项目说明文档
```

## 许可证

本项目采用 [MIT 许可证](https://opensource.org/licenses/MIT)授权。欢迎自由修改和分发！
