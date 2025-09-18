@echo off
chcp 65001 >nul
echo Compiling Gobang GUI version...
echo.

REM Check if SDL3 library exists
if not exist "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include" (
    echo Error: SDL3 library not found!
    echo Please ensure SDL3 is extracted to: D:\settings\SDL\SDL3-3.2.22\
    pause
    exit /b 1
)

REM Compile GUI version
gcc -std=c17 -o gobang_gui.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Generated: gobang_gui.exe
    echo.
    echo Copy SDL3.dll to current directory...
    copy "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\bin\SDL3.dll" .
    echo.
    echo Run GUI version:
    echo .\gobang_gui.exe
    echo.
) else (
    echo.
    echo Compilation failed! Please check:
    echo 1. SDL3 library path is correct
    echo 2. All source files exist
    echo 3. gcc compiler is installed
    echo.
)

pause