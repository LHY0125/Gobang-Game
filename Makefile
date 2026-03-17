# 五子棋游戏 Makefile
# 支持编译GUI版本 (IUP)

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
COMMON_SOURCES = $(SRC_DIR)/gobang.c $(SRC_DIR)/ai.c $(SRC_DIR)/config.c \
                 $(SRC_DIR)/globals.c \
                 $(SRC_DIR)/network.c $(SRC_DIR)/record.c $(SRC_DIR)/gui.c \
                 $(SRC_DIR)/gui_menu.c

# 目标文件 (src/xxx.c -> obj/xxx.o)
COMMON_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(COMMON_SOURCES))

# 可执行文件
GUI_TARGET = $(BIN_DIR)/gobang_gui.exe

# 默认目标
all: directories $(GUI_TARGET)

# 创建目录 (PowerShell 语法)
directories:
	if (!(Test-Path "$(OBJ_DIR)")) { New-Item -ItemType Directory -Path "$(OBJ_DIR)" | Out-Null }
	if (!(Test-Path "$(BIN_DIR)")) { New-Item -ItemType Directory -Path "$(BIN_DIR)" | Out-Null }

# GUI版本
$(GUI_TARGET): $(COMMON_OBJECTS) $(OBJ_DIR)/main.o
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -o $@ $^ $(IUP_LIBS) $(LDFLAGS)
	Copy-Item -Path "$(subst /,\,$(IUP_PATH))\iup.dll" -Destination "$(BIN_DIR)" -Force

# 通用目标文件编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -c -o $@ $<

# 编译 main.c
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(IUP_INCLUDE) -c -o $@ $<

# 清理规则 (PowerShell 语法)
clean:
	if (Test-Path "$(OBJ_DIR)") { Remove-Item -Path "$(OBJ_DIR)" -Recurse -Force }
	if (Test-Path "$(BIN_DIR)") { Remove-Item -Path "$(BIN_DIR)" -Recurse -Force }

# 编译GUI版本
gui: directories $(GUI_TARGET)

# 安装规则（可选）
install: all
	Write-Host "Installing executables..."
	Copy-Item -Path "$(GUI_TARGET)" -Destination "C:\Program Files\Gobang\" -Force

# 运行GUI版本
run-gui: $(GUI_TARGET)
	& ".\$(GUI_TARGET)"

# 帮助信息
help:
	@echo Available targets:
	@echo   all          - Build GUI version
	@echo   gui          - Build GUI version
	@echo   clean        - Remove all object files and executables
	@echo   run-gui      - Build and run GUI version
	@echo   install      - Install executables to system directory
	@echo   help         - Show this help message

# 声明伪目标
.PHONY: all clean gui install run-gui help directories
