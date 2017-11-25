#define MyAppName "Conference Server"

#include "all.iss"

[Setup]
AppId={{5DD6C0A0-0F14-46E6-A43A-39E61557869A}
UninstallRestartComputer=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: ServiceInstall; Description: "Install the Conference Server as Windows Service to automatically run with Windows startup."

[Files]
Source: "{#OCS_RELEASE_DIR_PATH}\server\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion
Source: "{#OCS_RELEASE_DIR_PATH}\server\default.ini.orig"; DestDir: "{app}"; DestName: "default.ini"; Flags: onlyifdoesntexist
Source: "{#OCS_RELEASE_DIR_PATH}\server\logging.conf.orig"; DestDir: "{app}"; DestName: "logging.conf"; Flags: onlyifdoesntexist

[Run]
Filename: "{app}\runtimes\vcredist_x86_2013.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable 2013 (32-bit)... This may take a while";
Filename: "{app}\runtimes\vcredist_x86_2015u3.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable 2015 Update 3 (32-bit)... This may take a while";
Filename: "{app}\runtimes\vcredist_x64_2013.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable (64-bit)... This may take a while"; Check: IsWin64;
Filename: "{app}\runtimes\vcredist_x64_2015u3.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable (64-bit)... This may take a while"; Check: IsWin64;

; ServerInstall task (Registers the videoserver.exe process as Windows service).
Filename: "{sys}\sc.exe"; Parameters: "create ""{#MyAppName}"" binPath= ""{app}\videoserver.exe --service"" start= auto"; Flags: runhidden; StatusMsg: "Installing as service..."; Tasks: ServiceInstall;
Filename: "{sys}\sc.exe"; Parameters: "start ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Starting {#MyAppName} service..."; Tasks: ServiceInstall;

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Stopping {#MyAppName} service..."; Tasks: ServiceInstall;
Filename: "{sys}\sc.exe"; Parameters: "delete ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Deleting {#MyAppName} service..."; Tasks: ServiceInstall;
