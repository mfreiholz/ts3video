@echo off
call "build-win64-env.bat"


echo --------------
echo --- SERVER ---
echo --------------

if not exist %OCS_RELEASE_DIR_PATH%\server mkdir %OCS_RELEASE_DIR_PATH%\server
if not exist %OCS_RELEASE_DIR_PATH%\server\runtimes mkdir %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2013.exe %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2015u3.exe %OCS_RELEASE_DIR_PATH%\server\runtimes


echo --------------
echo --- CLIENT ---
echo --------------

if not exist %OCS_RELEASE_DIR_PATH%\client mkdir %OCS_RELEASE_DIR_PATH%\client
if not exist %OCS_RELEASE_DIR_PATH%\client\runtimes mkdir %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2013.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2015u3.exe %OCS_RELEASE_DIR_PATH%\client\runtimes


echo ----------------
echo --- TS3VIDEO ---
echo ----------------

if not exist %OCS_RELEASE_DIR_PATH%\ts3video mkdir %OCS_RELEASE_DIR_PATH%\ts3video
if not exist %OCS_RELEASE_DIR_PATH%\ts3video\plugins mkdir %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_INSTALL_DIR_PATH%\ts3plugin\plugins\ts3video_win64.dll %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_INSTALL_DIR_PATH%\ts3plugin\package.ini %OCS_RELEASE_DIR_PATH%\ts3video

cd %OCS_RELEASE_DIR_PATH%\ts3video
rem cmake -E chdir "%OCS_RELEASE_DIR_PATH%\ts3video"
cmake -E tar "cfv" "ts3video.ts3_plugin" --format=zip "package.ini" "plugins"
cd %OCS_SOURCE_DIR%

echo ------------------
echo --- INSTALLERS ---
echo ------------------

iscc.exe innosetup\conference-server.iss
iscc.exe innosetup\conference-client.iss

pause