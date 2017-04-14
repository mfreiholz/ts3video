@echo off
call "build-win32-env.bat"
call "%VS140COMNTOOLS%\..\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"