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
Source: "{#OCS_DEPLOY_DIR_PATH}\server\*"; DestDir: "{app}"; Flags: recursesubdirs

[Run]
Filename: "{app}\vcredist_x86.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable (32-bit)... This may take a while";
Filename: "{sys}\sc.exe"; Parameters: "create ""{#MyAppName}"" binPath= ""{app}\videoserver.exe --service"" start= auto"; Flags: runhidden; StatusMsg: "Installing as service..."; Tasks: ServiceInstall;
Filename: "{sys}\sc.exe"; Parameters: "start ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Starting {#MyAppName} service..."; Tasks: ServiceInstall;

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Stopping {#MyAppName} service..."; Tasks: ServiceInstall;
Filename: "{sys}\sc.exe"; Parameters: "delete ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Deleting {#MyAppName} service..."; Tasks: ServiceInstall;
