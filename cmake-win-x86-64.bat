call "%VS120COMNTOOLS%\..\..\Vc\vcvarsall.bat" x64

cd /d %~dp0
if not exist build mkdir build
cd build
cmake -G "Visual Studio 12 Win64" -DCMAKE_INSTALL_PREFIX=install ..
