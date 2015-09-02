@echo off
call "%VS140COMNTOOLS%\..\..\Vc\vcvarsall.bat" x64

rem Prepare basic environment.
set MF_BUILD_DIR_NAME=build-win-x86-64
set MF_BUILD_DIR_PATH=%~dp0%MF_BUILD_DIR_NAME%
set QTDIR=D:\Development\Libraries\Qt\Online\5.3\msvc2013_64_opengl
set PATH=%PATH%;%QTDIR%\bin
