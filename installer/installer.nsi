; NSIS Installation Script
!include "MUI2.nsh"

; Basic Information
Name "Gobang Game"
OutFile "..\\Gobang_Setup.exe"
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
    
    ; Copy configuration and documentation files
    File "..\\gobang_config.ini"
    File "..\\README.md"
    
    ; Copy executable file if exists
    IfFileExists "..\\gobang.exe" 0 +2
    File "..\\gobang.exe"
    
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
