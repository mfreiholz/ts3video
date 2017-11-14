@echo off
call "build-win32-env.bat"

echo 
echo --- SERVER ---
if not exist %OCS_RELEASE_DIR_PATH%\server mkdir %OCS_RELEASE_DIR_PATH%\server
copy %OCS_BUILD_DIR_PATH%\projects\videoserver\Release\videoserver.exe %OCS_RELEASE_DIR_PATH%\server
copy %OCS_SOURCE_DIR%\projects\videoserver\res\default.ini %OCS_RELEASE_DIR_PATH%\server\default.ini.orig
copy %OCS_SOURCE_DIR%\projects\videoserver\res\logging.conf %OCS_RELEASE_DIR_PATH%\server\logging.conf.orig
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\server
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\server
windeployqt.exe %OCS_RELEASE_DIR_PATH%\server

echo 
echo --- CLIENT ---
if not exist %OCS_RELEASE_DIR_PATH%\client mkdir %OCS_RELEASE_DIR_PATH%\client
copy %OCS_BUILD_DIR_PATH%\projects\videoclient\Release\videoclient.exe %OCS_RELEASE_DIR_PATH%\client
copy %OCS_SOURCE_DIR%\projects\videoclient\res\default.ini %OCS_RELEASE_DIR_PATH%\client\default.ini.orig
copy %OCS_SOURCE_DIR%\projects\videoclient\res\logging.conf %OCS_RELEASE_DIR_PATH%\client\logging.conf.orig
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\client
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\client
windeployqt.exe %OCS_RELEASE_DIR_PATH%\client

echo 
echo --- TS3VIDEO ---
if not exist %OCS_RELEASE_DIR_PATH%\ts3video mkdir %OCS_RELEASE_DIR_PATH%\ts3video
copy %OCS_BUILD_DIR_PATH%\projects\ts3plugin\Release\ts3video_*.dll %OCS_RELEASE_DIR_PATH%\ts3video


pause