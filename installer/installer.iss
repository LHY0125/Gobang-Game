[Setup]
AppName=五子棋游戏
AppVersion=8.3
AppPublisher=LHY
AppPublisherURL=https://github.com/LHY0125/gobang.git
AppSupportURL=https://github.com/LHY0125/gobang.git
AppUpdatesURL=https://github.com/LHY0125/gobang.git
DefaultDirName={autopf}\Gobang
DefaultGroupName=五子棋游戏
AllowNoIcons=yes
LicenseFile=..\README.md
OutputDir=dist
OutputBaseFilename=Gobang_Inno_Setup
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\gobang_console.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\gobang_gui.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\SDL3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\include\*"; DestDir: "{app}\include"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\src\*"; DestDir: "{app}\src"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\MD\*"; DestDir: "{app}\MD"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\TXT\*"; DestDir: "{app}\TXT"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\records\*"; DestDir: "{app}\records"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\installer\*"; DestDir: "{app}\installer"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\compile.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\gobang_config.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Makefile"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\五子棋游戏(控制台版)"; Filename: "{app}\gobang_console.exe"
Name: "{group}\五子棋游戏(图形界面版)"; Filename: "{app}\gobang_gui.exe"
Name: "{group}\{cm:UninstallProgram,五子棋游戏}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\五子棋游戏"; Filename: "{app}\gobang_gui.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\gobang_gui.exe"; Description: "{cm:LaunchProgram,五子棋游戏}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\records"
Type: filesandordirs; Name: "{app}\include"
Type: filesandordirs; Name: "{app}\src"
Type: filesandordirs; Name: "{app}\MD"
Type: filesandordirs; Name: "{app}\TXT"
Type: filesandordirs; Name: "{app}\installer"