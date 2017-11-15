@echo off
call "build-win32-env.bat"

echo 
echo --- SERVER ---
if not exist %OCS_RELEASE_DIR_PATH%\server mkdir %OCS_RELEASE_DIR_PATH%\server
copy %OCS_BUILD_DIR_PATH%\projects\videoserver\Release\videoserver.exe %OCS_RELEASE_DIR_PATH%\server
copy %OCS_SOURCE_DIR%\projects\videoserver\res\default.ini %OCS_RELEASE_DIR_PATH%\server\default.ini.orig
copy %OCS_SOURCE_DIR%\projects\videoserver\res\logging.conf %OCS_RELEASE_DIR_PATH%\server\logging.conf.orig
if not exist %OCS_RELEASE_DIR_PATH%\server\runtimes mkdir %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\server\runtimes
windeployqt.exe --no-compiler-runtime --release %OCS_RELEASE_DIR_PATH%\server\videoserver.exe


echo 
echo --- CLIENT ---
if not exist %OCS_RELEASE_DIR_PATH%\client mkdir %OCS_RELEASE_DIR_PATH%\client
copy %OCS_BUILD_DIR_PATH%\projects\videoclient\Release\videoclient.exe %OCS_RELEASE_DIR_PATH%\client
copy %OCS_SOURCE_DIR%\projects\videoclient\res\default.ini %OCS_RELEASE_DIR_PATH%\client\default.ini.orig
copy %OCS_SOURCE_DIR%\projects\videoclient\res\logging.conf %OCS_RELEASE_DIR_PATH%\client\logging.conf.orig
if not exist %OCS_RELEASE_DIR_PATH%\client\runtimes mkdir %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2013.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2015u3.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
windeployqt.exe --no-compiler-runtime --release %OCS_RELEASE_DIR_PATH%\client\videoclient.exe


echo 
echo --- TS3VIDEO ---
if not exist %OCS_RELEASE_DIR_PATH%\ts3video mkdir %OCS_RELEASE_DIR_PATH%\ts3video
if not exist %OCS_RELEASE_DIR_PATH%\ts3video\plugins mkdir %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_BUILD_DIR_PATH%\projects\ts3plugin\Release\ts3video_win32.dll %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_BUILD_DIR_PATH%\deploy\ts3plugin\package.ini %OCS_RELEASE_DIR_PATH%\ts3video


pause