@echo off
call "%VS120COMNTOOLS%\..\..\Vc\vcvarsall.bat" x64

set MF_BUILD_DIR_NAME=build-win-x86-64
set MF_BUILD_DIR_PATH=%~dp0%MF_BUILD_DIR_NAME%
set QTDIR=D:\Development\Libraries\Qt\Online\5.4\msvc2013_64_opengl
set PATH=%PATH%;%QTDIR%\bin

rem Print build environment settings.
echo.
echo Build environment
echo     MF_BUILD_DIR_NAME: %MF_BUILD_DIR_NAME%
echo     MF_BUILD_DIR_PATH: %MF_BUILD_DIR_PATH%
echo     QTDIR: %QTDIR%
echo.
pause

rem Create and enter build directory.
cd /d %~dp0
if not exist %MF_BUILD_DIR_PATH% mkdir %MF_BUILD_DIR_PATH%
cd %MF_BUILD_DIR_PATH%

rem Configure with CMake.
cmake -G "Visual Studio 12 Win64" -DCMAKE_INSTALL_PREFIX=install ..

pause