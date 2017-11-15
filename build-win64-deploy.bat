@echo off
call "build-win64-env.bat"


echo 
echo --- TS3VIDEO ---
copy %OCS_BUILD_DIR_PATH%\projects\ts3plugin\Release\ts3video_win64.dll %OCS_RELEASE_DIR_PATH%\ts3video\plugins
copy %OCS_BUILD_DIR_PATH%\deploy\ts3plugin\package.ini %OCS_RELEASE_DIR_PATH%\ts3video

cd %OCS_RELEASE_DIR_PATH%\ts3video
rem cmake -E chdir "%OCS_RELEASE_DIR_PATH%\ts3video"
cmake -E tar "cfv" "ts3video.ts3_plugin" --format=zip "package.ini" "plugins"


pause