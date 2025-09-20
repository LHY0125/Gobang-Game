@echo off
echo ===== Gobang Game Compile Script =====
echo.
echo Please select compile version:
echo 1. Console version (gobang_console.exe)
echo 2. GUI version (gobang_gui.exe)
echo 3. Compile all versions
echo 0. Exit
echo.
set /p choice="Please enter your choice (0-3): "

if "%choice%"=="0" goto :exit
if "%choice%"=="1" goto :console
if "%choice%"=="2" goto :gui
if "%choice%"=="3" goto :all
echo Invalid choice!
pause
exit /b 1

:console
echo.
echo Compiling console version...
echo.
gcc -std=c17 -o gobang_console.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Generated: gobang_console.exe
    echo Run command: .\gobang_console.exe
    echo.
) else (
    echo.
    echo Compilation failed! Please check source files and compiler installation
    echo.
)
goto :end

:gui
echo.
echo Compiling GUI version...
echo.
REM Check if SDL3 library exists
if not exist "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include" (
    echo Error: SDL3 library not found!
    echo Please ensure SDL3 is extracted to: D:\settings\SDL\SDL3-3.2.22\
    goto :end
)
gcc -std=c17 -o gobang_gui.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Generated: gobang_gui.exe
    echo.
    echo Copying SDL3.dll to current directory...
    copy "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\bin\SDL3.dll" . >nul 2>&1
    echo Run command: .\gobang_gui.exe
    echo.
) else (
    echo.
    echo Compilation failed! Please check:
    echo 1. SDL3 library path is correct
    echo 2. All source files exist
    echo 3. gcc compiler is installed
    echo.
)
goto :end

:all
echo.
echo Compiling all versions...
echo.
echo [1/2] Compiling console version...
gcc -std=c17 -o gobang_console.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo Console version compilation successful!
) else (
    echo Console version compilation failed!
)
echo.
echo [2/2] Compiling GUI version...
if not exist "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include" (
    echo Error: SDL3 library not found! Skipping GUI version compilation
    goto :end
)
gcc -std=c17 -o gobang_gui.exe *.c -ID:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\include -LD:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\lib -lSDL3 -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo GUI version compilation successful!
    copy "D:\settings\SDL\SDL3-3.2.22\x86_64-w64-mingw32\bin\SDL3.dll" . >nul 2>&1
) else (
    echo GUI version compilation failed!
)
echo.
echo Compilation complete! Generated files:
dir *.exe 2>nul
echo.
goto :end

:end
pause

:exit