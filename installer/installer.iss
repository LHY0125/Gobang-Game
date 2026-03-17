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
Source: "..\bin\gobang_gui.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\iup.dll"; DestDir: "{app}"; Flags: ignoreversion
; 允许文件夹为空时不报错
Source: "..\bin\records\*"; DestDir: "{app}\records"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\五子棋游戏(图形界面版)"; Filename: "{app}\gobang_gui.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,五子棋游戏}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\五子棋游戏"; Filename: "{app}\gobang_gui.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\gobang_gui.exe"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,五子棋游戏}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\records"
Type: files; Name: "{app}\gobang_gui.exe"
Type: files; Name: "{app}\iup.dll"
Type: dirifempty; Name: "{app}"
