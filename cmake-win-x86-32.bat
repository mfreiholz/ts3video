call "%VS120COMNTOOLS%\..\..\Vc\vcvarsall.bat" x86

cd /d %~dp0
if not exist build mkdir build
cd build
cmake -G "Visual Studio 12" -DCMAKE_INSTALL_PREFIX=install ..
