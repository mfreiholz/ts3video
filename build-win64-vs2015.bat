@echo off
call "build-win64-env.bat"
call "%VS120COMNTOOLS%\..\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"