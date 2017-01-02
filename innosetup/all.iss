; Environment variables as constants
#define OCS_DEPLOY_DIR_PATH GetEnv('OCS_DEPLOY_DIR_PATH')
#define OCS_SETUP_DIR_PATH GetEnv('OCS_SETUP_DIR_PATH')

; Publisher information
#define MyOrgName "M. Freiholz Software Development"
#define MyOrgDomain "mfreiholz.de"
#define MyOrgURL "https://mfreiholz.de"

; Application information
#define MyAppURL "https://mfreiholz.de/ts3video"
#define MyAppVersion "0.10"

[Setup]
AppPublisher={#MyOrgName}
AppPublisherURL={#MyOrgURL}
VersionInfoCompany={#MyOrgName}

AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

DefaultDirName={pf}\{#MyOrgDomain}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes

SetupLogging=yes
Compression=lzma
SolidCompression=yes

OutputDir={#OCS_SETUP_DIR_PATH}
OutputBaseFilename={#MyAppName} {#MyAppVersion} Setup

; Comsmetics
AppCopyright=Copyright (C) 2015-2017 {#MyOrgName}

; Basic registry values for all applications.
[Registry]
; Store the absolute path to the application's install location.
; Note: Windows might store it in the "WOW6432Node" folder: "Software\WOW6432Node\mfreiholz.de\appname"
Root: HKLM; Subkey: "Software\{#MyOrgDomain}\{#MyAppName}"; ValueType: string; ValueData: "{app}"; Flags: uninsdeletekey;
