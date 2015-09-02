@echo off
call "env-win-x86-32.bat"

rem Create and enter build directory.
cd /d %~dp0
if not exist %MF_BUILD_DIR_PATH% mkdir %MF_BUILD_DIR_PATH%
cd %MF_BUILD_DIR_PATH%

rem Configure with CMake.
cmake -G "Visual Studio 12" -DCMAKE_INSTALL_PREFIX=install ..

pause