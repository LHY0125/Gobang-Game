; NSIS Installation Script
!include "MUI2.nsh"

; Basic Information
Name "Gobang Game"
OutFile "Gobang_Setup.exe"
InstallDir "$PROGRAMFILES\Gobang"
RequestExecutionLevel admin

; Interface Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"

; Installation Pages
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Language Settings
!insertmacro MUI_LANGUAGE "SimpChinese"

; Installation Section
Section "Main"
    SetOutPath "$INSTDIR"
    
    ; Copy all C source files
    File "ai.c"
    File "config.c"
    File "game_mode.c"
    File "globals.c"
    File "gobang.c"
    File "init_board.c"
    File "main.c"
    File "network.c"
    File "record.c"
    File "ui.c"
    
    ; Copy all header files
    File "ai.h"
    File "config.h"
    File "game_mode.h"
    File "globals.h"
    File "gobang.h"
    File "init_board.h"
    File "network.h"
    File "record.h"
    File "type.h"
    File "ui.h"
    
    ; Copy configuration and documentation files
    File "gobang_config.ini"
    File "README.md"
    
    ; Copy directories
    File /r "MD"
    File /r "TXT"
    
    ; Copy executable file if exists
    IfFileExists "gobang.exe" 0 +2
    File "gobang.exe"
    
    ; Create program group directory
    CreateDirectory "$SMPROGRAMS\Gobang"
    
    ; Create shortcuts (only if executable exists)
    IfFileExists "$INSTDIR\gobang.exe" 0 +3
    CreateShortCut "$DESKTOP\Gobang.lnk" "$INSTDIR\gobang.exe"
    CreateShortCut "$SMPROGRAMS\Gobang\Gobang.lnk" "$INSTDIR\gobang.exe"
    
    ; Write uninstall information
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Gobang" \
                     "DisplayName" "Gobang Game"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Gobang" \
                     "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
SectionEnd

; Uninstall Section
Section "Uninstall"
    RMDir /r "$INSTDIR"
    Delete "$DESKTOP\Gobang.lnk"
    RMDir /r "$SMPROGRAMS\Gobang"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Gobang"
SectionEnd