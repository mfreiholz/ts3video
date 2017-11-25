@echo off
call "build-win32-env.bat"

rem Create and enter build directory.
cd /d %~dp0
if not exist %OCS_BUILD_DIR_PATH% mkdir %OCS_BUILD_DIR_PATH%
cd %OCS_BUILD_DIR_PATH%

rem Configure with CMake.
cmake -G "Visual Studio 14 2015" -DCMAKE_INSTALL_PREFIX="%OCS_INSTALL_DIR_PATH%" ..

pause