# 五子棋游戏编译指南

本项目现在支持使用 `make` 命令进行编译，提供了更便捷的构建方式。

## 系统要求

- MinGW64 编译器
- SDL3 开发库（用于图形界面）
- Make 工具

## 编译命令

### 查看所有可用命令
```bash
make help
```

### 编译所有版本
```bash
make all
```
这将同时编译控制台版本和GUI版本。

### 只编译控制台版本
```bash
make console
```
生成 `gobang_console.exe`

### 只编译GUI版本
```bash
make gui
```
生成 `gobang_gui.exe`

### 清理编译文件
```bash
make clean
```
删除所有目标文件(.o)和可执行文件(.exe)

### 编译并运行
```bash
# 编译并运行控制台版本
make run-console

# 编译并运行GUI版本
make run-gui
```

## 生成的文件

- `gobang_console.exe` - 控制台版本，支持所有功能包括图形界面模式
- `gobang_gui.exe` - GUI版本，与控制台版本功能相同

## 注意事项

1. 两个版本都包含完整功能，包括图形界面支持
2. 需要确保SDL3库路径正确配置在Makefile中
3. 编译时会显示一些警告，这是正常的
4. 如果遇到编译错误，请检查MinGW64和SDL3的安装路径

## 传统编译方式

如果不使用Makefile，仍可以使用传统的gcc命令：

```bash
# 控制台版本
gcc -std=c17 -o gobang.exe *.c -lws2_32

# GUI版本
gcc -std=c17 -o gobang_gui.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
```