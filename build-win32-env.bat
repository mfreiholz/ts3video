@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86

rem Build specific settings
set OCS_BUILD_DIR_NAME=build-win-x86-32

rem Setup Qt
set QTDIR=%OCS_QTDIR_X86_32%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTDIR%\plugins
set PATH=%QTDIR%\bin;%PATH%

call "build-win-env.bat"