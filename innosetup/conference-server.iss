#define MyAppName "VideoServer"

#include "all.iss"

[Setup]
AppId={{5DD6C0A0-0F14-46E6-A43A-39E61557869A}
OutputBaseFilename={#MyAppName}-{#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#OCS_DEPLOY_DIR_PATH}\server\*"; DestDir: "{app}"; Flags: recursesubdirs

[Run]
Filename: "{app}\vcredist_x86.exe"; Parameters: "/install /quiet"; StatusMsg: "Installing VC Redistributable..."
Filename: "{sys}\sc.exe"; Parameters: "create ""{#MyAppName}"" binPath= ""{app}\videoserver.exe --service"" start= auto"; Flags: runhidden; StatusMsg: "Installing as service..."

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Stopping {#MyAppName} service..."
Filename: "{sys}\sc.exe"; Parameters: "delete ""{#MyAppName}"""; Flags: runhidden; StatusMsg: "Deleting {#MyAppName} service..."
