@echo off
call "%VS140COMNTOOLS%\..\..\Vc\vcvarsall.bat" x86

rem Build specific settings
set OCS_BUILD_DIR_NAME=build-win-x86-32

rem Setup Qt
set QTDIR=%~dp0..\ocs-qt\qt-5.5.1-x86-32-vc14-opengl-openssl-build
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTDIR%\plugins
set PATH=%PATH%;%QTDIR%\bin

call "env-win-all.bat"