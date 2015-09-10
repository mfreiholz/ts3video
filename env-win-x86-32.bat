@echo off
call "%VS140COMNTOOLS%\..\..\Vc\vcvarsall.bat" x86

rem Prepare basic environment.
set MF_BUILD_DIR_NAME=build-win-x86-32
set MF_BUILD_DIR_PATH=%~dp0%MF_BUILD_DIR_NAME%
set QTDIR=D:\Development\Libraries\Qt\Qt-5.4.1-x86-32-OpenGL\5.4\msvc2013_opengl
set PATH=%PATH%;%QTDIR%\bin
