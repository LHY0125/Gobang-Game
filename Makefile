# 五子棋游戏 Makefile
# 支持编译控制台版本和GUI版本 (IUP)

# 编译器设置
CC = gcc
# 显式指定 Shell 为 PowerShell
SHELL = D:/PowerShell/PowerShell-7.5.4/PowerShell.exe
.SHELLFLAGS = -NoProfile -Command

CFLAGS = -Wall -Wextra -std=c17 -O2 -Iinclude -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = -lws2_32

# IUP路径设置
IUP_PATH = libs/iup-3.31_Win64_dllw6_lib
IUP_INCLUDE = "-I$(IUP_PATH)/include"
# IUP链接库: iup, gdi32, comdlg32, comctl32, uuid, ole32
IUP_LIBS = "-L$(IUP_PATH)" -liup -lgdi32 -lcomdlg32 -lcomctl32 -luuid -lole32

# 目录设置
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 源文件
COMMON_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/gobang.c $(SRC_DIR)/ai.c $(SRC_DIR)/config.c \
                 $(SRC_DIR)/game_mode.c $(SRC_DIR)/globals.c $(SRC_DIR)/init_board.c \
                 $(SRC_DIR)/network.c $(SRC_DIR)/record.c $(SRC_DIR)/ui.c $(SRC_DIR)/gui.c \
                 $(SRC_DIR)/gui_menu.c

GUI_SOURCES = $(COMMON_SOURCES)
CONSOLE_SOURCES = $(COMMON_SOURCES)

# 目标文件 (src/xxx.c -> obj/xxx.o)
COMMON_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(COMMON_SOURCES))

# 可执行文件
CONSOLE_TARGET = $(BIN_DIR)/gobang_console.exe
GUI_TARGET = $(BIN_DIR)/gobang_gui.exe

# 默认目标
all: directories $(CONSOLE_TARGET) $(GUI_TARGET)

# 创建目录 (PowerShell 语法)
directories:
	if (!(Test-Path "$(OBJ_DIR)")) { New-Item -ItemType Directory -Path "$(OBJ_DIR)" | Out-Null }
	if (!(Test-Path "$(BIN_DIR)")) { New-Item -ItemType Directory -Path "$(BIN_DIR)" | Out-Null }

# 控制台版本
$(CONSOLE_TARGET): $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -o $@ $^ $(IUP_LIBS) $(LDFLAGS)

# GUI版本
$(GUI_TARGET): $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -o $@ $^ $(IUP_LIBS) $(LDFLAGS)
	Copy-Item -Path "$(subst /,\,$(IUP_PATH))\iup.dll" -Destination "$(BIN_DIR)" -Force

# 通用目标文件编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -c -o $@ $<

# 清理规则 (PowerShell 语法)
clean:
	if (Test-Path "$(OBJ_DIR)") { Remove-Item -Path "$(OBJ_DIR)" -Recurse -Force }
	if (Test-Path "$(BIN_DIR)") { Remove-Item -Path "$(BIN_DIR)" -Recurse -Force }

# 只编译控制台版本
console: directories $(CONSOLE_TARGET)

# 只编译GUI版本
gui: directories $(GUI_TARGET)

# 安装规则（可选）
install: all
	Write-Host "Installing executables..."
	Copy-Item -Path "$(CONSOLE_TARGET)" -Destination "C:\Program Files\Gobang\" -Force
	Copy-Item -Path "$(GUI_TARGET)" -Destination "C:\Program Files\Gobang\" -Force

# 运行控制台版本
run-console: $(CONSOLE_TARGET)
	& ".\$(CONSOLE_TARGET)"

# 运行GUI版本
run-gui: $(GUI_TARGET)
	& ".\$(GUI_TARGET)"

# 帮助信息
help:
	@echo Available targets:
	@echo   all          - Build both console and GUI versions
	@echo   console      - Build console version only
	@echo   gui          - Build GUI version only
	@echo   clean        - Remove all object files and executables
	@echo   run-console  - Build and run console version
	@echo   run-gui      - Build and run GUI version
	@echo   install      - Install executables to system directory
	@echo   help         - Show this help message

# 声明伪目标
.PHONY: all clean console gui install run-console run-gui help directories
