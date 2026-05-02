cmake_minimum_required(VERSION 3.16)
project(Gobang VERSION 9.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# === 输出目录：与原 Makefile 保持一致 ===
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# === 编译选项 ===
add_compile_options(-Wall -Wextra -O2)
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# === ENet（从源码编译静态库）===
set(ENET_DIR ${CMAKE_SOURCE_DIR}/libs/ENET)
add_library(enet STATIC
    ${ENET_DIR}/callbacks.c
    ${ENET_DIR}/compress.c
    ${ENET_DIR}/host.c
    ${ENET_DIR}/list.c
    ${ENET_DIR}/packet.c
    ${ENET_DIR}/peer.c
    ${ENET_DIR}/protocol.c
    ${ENET_DIR}/win32.c
)
target_include_directories(enet PUBLIC ${ENET_DIR}/include)

# === IUP（预构建导入库）===
set(IUP_DIR ${CMAKE_SOURCE_DIR}/libs/IUP)
add_library(iup SHARED IMPORTED)
set_target_properties(iup PROPERTIES
    IMPORTED_IMPLIB   ${IUP_DIR}/libiup.a
    IMPORTED_LOCATION ${IUP_DIR}/iup.dll
    INTERFACE_INCLUDE_DIRECTORIES ${IUP_DIR}/include
)

# === cJSON（JSON处理库）===
add_library(cjson STATIC ${CMAKE_SOURCE_DIR}/libs/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${CMAKE_SOURCE_DIR}/libs/cJSON)

# === 主可执行文件 ===
add_executable(gobang_gui
    src/core/main.c
    src/core/globals.c
    src/core/config.c
    src/core/gobang.c
    src/core/ai.c
    src/network/network.c
    src/record/record.c
    src/gui/gui_core.c
    src/gui/gui_game.c
    src/gui/gui_menu.c
    src/gui/gui_replay.c
    src/llm/llm_ai.c
)

target_include_directories(gobang_gui PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(gobang_gui PRIVATE
    enet
    cjson
    iup
    winhttp
    ws2_32
    winmm
    gdi32
    comdlg32
    comctl32
    uuid
    ole32
)

# === 构建后拷贝 iup.dll 到 bin/ ===
add_custom_command(TARGET gobang_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${IUP_DIR}/iup.dll
        $<TARGET_FILE_DIR:gobang_gui>
    COMMENT "拷贝 iup.dll 到输出目录"
)

# === 快捷运行目标 ===
add_custom_target(run
    COMMAND gobang_gui
    DEPENDS gobang_gui
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    COMMENT "运行五子棋游戏"
)
