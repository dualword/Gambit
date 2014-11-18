; Written by Jelle Geerts (jellegeerts@gmail.com).
;
; To the extent possible under law, the author(s) have dedicated all
; copyright and related and neighboring rights to this software to
; the public domain worldwide. This software is distributed without
; any warranty.
;
; You should have received a copy of the CC0 Public Domain Dedication
; along with this software.
; If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

; Inno Setup script.

#define MyAppName             "Gambit"
; MyAppVersion is defined by the script that generates the Inno Setup
; script from _this_ template file.
;#define MyAppVersion          "0.0"
#define MyAppPublisher        "Jelle Geerts"
#define MyAppURL              "http://purl.org/net/gambit"
#define MyAppExeName          "gambitchess.exe"
#define MyAppShortName        "Gambit"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2982D55D-D482-4D8C-82D3-435595DB092D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
CloseApplications=yes
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename={#MyAppShortName}-{#MyAppVersion}-Win-setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Gambit\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Gambit\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

