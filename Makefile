# 五子棋游戏 Makefile
# 支持编译控制台版本和GUI版本

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2
LDFLAGS = -lws2_32

# SDL3路径设置
SDL3_PATH = D:/settings/SDL/SDL3-3.2.22/x86_64-w64-mingw32
SDL3_INCLUDE = -I$(SDL3_PATH)/include
SDL3_LIBS = -L$(SDL3_PATH)/lib -lSDL3 -lmingw32

# 源文件
COMMON_SOURCES = main.c gobang.c ai.c config.c game_mode.c globals.c \
                 init_board.c network.c record.c ui.c gui.c type.h

GUI_SOURCES = $(COMMON_SOURCES)
CONSOLE_SOURCES = $(COMMON_SOURCES)

# 目标文件
COMMON_OBJECTS = $(patsubst %.c,%.o,$(filter %.c,$(COMMON_SOURCES)))

# 可执行文件
CONSOLE_TARGET = gobang_console.exe
GUI_TARGET = gobang_gui.exe

# 默认目标
all: $(CONSOLE_TARGET) $(GUI_TARGET)

# 控制台版本
$(CONSOLE_TARGET): $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) $(SDL3_INCLUDE) -o $@ $^ $(SDL3_LIBS) $(LDFLAGS)

# GUI版本
$(GUI_TARGET): $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) $(SDL3_INCLUDE) -o $@ $^ $(SDL3_LIBS) $(LDFLAGS)

# 通用目标文件编译规则（包含SDL3头文件路径，因为多个文件包含gui.h）
%.o: %.c
	$(CC) $(CFLAGS) $(SDL3_INCLUDE) -c -o $@ $<

# 清理规则
clean:
	del /Q *.o *.exe 2>nul || true

# 只编译控制台版本
console: $(CONSOLE_TARGET)

# 只编译GUI版本
gui: $(GUI_TARGET)

# 安装规则（可选）
install: all
	@echo Installing executables...
	copy $(CONSOLE_TARGET) C:\Program Files\Gobang\ 2>nul || echo Install directory not found
	copy $(GUI_TARGET) C:\Program Files\Gobang\ 2>nul || echo Install directory not found

# 运行控制台版本
run-console: $(CONSOLE_TARGET)
	.\$(CONSOLE_TARGET)

# 运行GUI版本
run-gui: $(GUI_TARGET)
	.\$(GUI_TARGET)

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
.PHONY: all clean console gui install run-console run-gui help