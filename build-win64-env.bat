@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rem Build specific settings
set OCS_BUILD_DIR_NAME=build-win-x86-64

rem Setup Qt
set OCS_QTDIR_X86_64=E:\Qt\5.15.1\msvc2019_64
set QTDIR=%OCS_QTDIR_X86_64%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTDIR%\plugins
set PATH=%QTDIR%\bin;%PATH%

call "build-win-env.bat"