@echo off
call "build-win64-env.bat"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" "%OCS_BUILD_DIR_PATH%\source.sln"