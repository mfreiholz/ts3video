; Environment variables as constants
#define OCS_DEPLOY_DIR_PATH GetEnv('OCS_DEPLOY_DIR_PATH')
#define OCS_SETUP_DIR_PATH GetEnv('OCS_SETUP_DIR_PATH')

; Basic constants for all setups
#define MyOrgDomain "mfreiholz.de"
#define MyOrgName "M. Freiholz Software Development"

#define MyAppVersion "0.6"
#define MyAppURL "https://mfreiholz.de/ts3video"

[Setup]
AppPublisher={#MyOrgName}
AppPublisherURL={#MyAppURL}
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

; Comsmetics
AppCopyright=Copyright (C) 2015-2016 {#MyOrgName}
