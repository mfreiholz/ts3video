#define MyAppName "Conference Client"

#include "all.iss"

[Setup]
AppId={{6186D48B-32FB-4E48-9085-ACC07BA5FB0F}
InfoBeforeFile=conference-client_infobeforefile_english.rtf
InfoAfterFile=conference-client_infoafterfile_english.rtf

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#OCS_RELEASE_DIR_PATH}\client\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion
Source: "{#OCS_RELEASE_DIR_PATH}\client\default.ini.orig"; DestDir: "{app}"; DestName: "default.ini"; Flags: onlyifdoesntexist
Source: "{#OCS_RELEASE_DIR_PATH}\client\logging.conf.orig"; DestDir: "{app}"; DestName: "logging.conf"; Flags: onlyifdoesntexist
Source: "{#OCS_RELEASE_DIR_PATH}\ts3video\ts3video.ts3_plugin"; DestDir: "{app}"; Flags: ignoreversion

[Run]
Filename: "{app}\runtimes\vcredist_x86_2013.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable 2013 (32-bit)... This may take a while";
Filename: "{app}\runtimes\vcredist_x86_2015u3.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable 2015 Update 3 (32-bit)... This may take a while";
Filename: "{app}\runtimes\vcredist_x64_2013.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable (64-bit)... This may take a while"; Check: IsWin64;
Filename: "{app}\runtimes\vcredist_x64_2015u3.exe"; Parameters: "/install /quiet"; Flags: skipifdoesntexist; StatusMsg: "Installing VC Redistributable (64-bit)... This may take a while"; Check: IsWin64;
Filename: "{app}\ts3video.ts3_plugin"; StatusMsg: "Installing TeamSpeak 3 Plugin..."; Flags: shellexec waituntilterminated
