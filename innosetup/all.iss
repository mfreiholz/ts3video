; Environment variables as constants
#define OCS_DEPLOY_DIR_PATH GetEnv('OCS_DEPLOY_DIR_PATH')
#define OCS_SETUP_DIR_PATH GetEnv('OCS_SETUP_DIR_PATH')

[Setup]
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
SetupLogging=yes
Compression=lzma
SolidCompression=yes
OutputDir={#OCS_SETUP_DIR_PATH}
