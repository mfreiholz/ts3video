#define MyAppName "Conference Client"

#include "all.iss"

[Setup]
AppId={{6186D48B-32FB-4E48-9085-ACC07BA5FB0F}
OutputBaseFilename={#MyAppName} {#MyAppVersion} Setup
InfoBeforeFile=conference-client_infobeforefile_english.rtf
InfoAfterFile=conference-client_infoafterfile_english.rtf

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#OCS_DEPLOY_DIR_PATH}\client\*"; DestDir: "{app}"; Flags: recursesubdirs
Source: "{#OCS_DEPLOY_DIR_PATH}\ts3plugin\*.ts3_plugin"; DestDir: "{app}";

[Run]
Filename: "{app}\vcredist_x86.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable..."
Filename: "{app}\ts3video.ts3_plugin"; StatusMsg: "Installing TeamSpeak 3 Plugin..."; Flags: shellexec waituntilterminated

[Registry]
Root: HKLM; Subkey: "Software\{#MyOrgDomain}\{#MyAppName}"; ValueType: string; ValueData: "{app}"; Flags: uninsdeletekey
