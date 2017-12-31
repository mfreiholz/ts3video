@echo off
call "build-win32-env.bat"
call "%VS120COMNTOOLS%\..\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"
rem call "%VS140COMNTOOLS%\..\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"