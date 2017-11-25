@echo off

rem List of paths
set OCS_SOURCE_DIR=%~dp0
set OCS_BUILD_DIR_PATH=%~dp0%OCS_BUILD_DIR_NAME%
set OCS_INSTALL_DIR_PATH=%OCS_BUILD_DIR_PATH%\install

rem Same for 32-bit and 64-bit
set OCS_RELEASE_DIR_PATH=%OCS_SOURCE_DIR%\RELEASE_DIR

rem Extend PATH for InnoSetup-Compiler
set PATH=C:\Program Files (x86)\Inno Setup 5;%PATH%

rem Print used environment
echo
echo OCS Build Environment
echo ---------------------
echo OCS_SOURCE_DIR       : %OCS_SOURCE_DIR%
echo OCS_BUILD_DIR_PATH   : %OCS_BUILD_DIR_PATH%
echo OCS_INSTALL_DIR_PATH : %OCS_INSTALL_DIR_PATH%
echo OCS_RELEASE_DIR_PATH : %OCS_RELEASE_DIR_PATH%
echo QTDIR                : %QTDIR%
echo PATH                 : %PATH%
echo ---------------------
echo 
