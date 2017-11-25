@echo off
call "build-win32-env.bat"


echo --------------
echo --- SERVER ---
echo --------------

if not exist %OCS_RELEASE_DIR_PATH%\server mkdir %OCS_RELEASE_DIR_PATH%\server
xcopy %OCS_INSTALL_DIR_PATH%\server\* %OCS_RELEASE_DIR_PATH%\server /H /Y /I

if not exist %OCS_RELEASE_DIR_PATH%\server\runtimes mkdir %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\server\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\server\runtimes

windeployqt.exe --no-compiler-runtime --release %OCS_RELEASE_DIR_PATH%\server\videoserver.exe


echo --------------
echo --- CLIENT ---
echo --------------

if not exist %OCS_RELEASE_DIR_PATH%\client mkdir %OCS_RELEASE_DIR_PATH%\client
xcopy %OCS_INSTALL_DIR_PATH%\client\* %OCS_RELEASE_DIR_PATH%\client /H /Y /I

if not exist %OCS_RELEASE_DIR_PATH%\client\runtimes mkdir %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2013.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x86_2015u3.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
rem copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2013.exe %OCS_RELEASE_DIR_PATH%\client\runtimes
rem copy %OCS_SOURCE_DIR%\..\ocs-vendor-runtimes\vcredist_x64_2015u3.exe %OCS_RELEASE_DIR_PATH%\client\runtimes

windeployqt.exe --no-compiler-runtime --release %OCS_RELEASE_DIR_PATH%\client\videoclient.exe


echo ----------------
echo --- TS3VIDEO ---
echo ----------------

if not exist %OCS_RELEASE_DIR_PATH%\ts3video mkdir %OCS_RELEASE_DIR_PATH%\ts3video
if not exist %OCS_RELEASE_DIR_PATH%\ts3video\plugins mkdir %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_INSTALL_DIR_PATH%\ts3plugin\plugins\ts3video_win32.dll %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_INSTALL_DIR_PATH%\ts3plugin\package.ini %OCS_RELEASE_DIR_PATH%\ts3video

pause