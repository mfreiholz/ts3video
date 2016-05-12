@echo off
call "env-win-x86-64.bat"
call "%VS140COMNTOOLS%\..\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"