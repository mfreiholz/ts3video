@echo off
call "env-win-x86-64.bat"

rem Create and enter build directory.
cd /d %~dp0
if not exist %OCS_BUILD_DIR_PATH% mkdir %OCS_BUILD_DIR_PATH%
cd %OCS_BUILD_DIR_PATH%

rem Configure with CMake.
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX="%OCS_DEPLOY_DIR_PATH%" ..
rem cmake -G "Visual Studio 12 2013 Win64" -DCMAKE_INSTALL_PREFIX="%OCS_DEPLOY_DIR_PATH%" ..

pause