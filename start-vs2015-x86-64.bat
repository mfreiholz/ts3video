@echo off
call "env-win-x86-64.bat"

rem Run Visual Studio 2015
call "%VS140COMNTOOLS%\..\IDE\devenv.exe" build-win-x86-64/source.sln