@echo off

rem List of paths
set OCS_SOURCE_DIR=%~dp0
set OCS_BUILD_DIR_PATH=%~dp0%OCS_BUILD_DIR_NAME%
set OCS_DEPLOY_DIR_PATH=%OCS_BUILD_DIR_PATH%\deploy

rem Same for 32-bit and 64-bit
set OCS_RELEASE_DIR_PATH=%OCS_SOURCE_DIR%\RELEASE_DIR
set OCS_SETUP_DIR_PATH=%OCS_SOURCE_DIR%\RELEASE_DIR\setup

rem Print used environment
echo
echo OCS Build Environment
echo ---------------------
echo OCS_SOURCE_DIR      : %OCS_SOURCE_DIR%
echo OCS_BUILD_DIR_PATH  : %OCS_BUILD_DIR_PATH%
echo OCS_DEPLOY_DIR_PATH : %OCS_DEPLOY_DIR_PATH%
echo OCS_RELEASE_DIR_PATH: %OCS_RELEASE_DIR_PATH%
echo OCS_SETUP_DIR_PATH  : %OCS_SETUP_DIR_PATH%
echo QTDIR               : %QTDIR%
echo ---------------------
echo 
