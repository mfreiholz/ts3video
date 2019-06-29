![Logo](projects/videoclient/res/logo-48x48.png)

# Build

Build has been tested on

- Windows 8.1 / 10
- Ubuntu 14.04
- Ubuntu 15.10

## Requirements

- CMake >= 3.0.0
- Qt 5.4.1 build with OpenGL
	- https://download.qt.io/archive/qt/5.5/5.5.1/qt-opensource-windows-x86-msvc2013-5.5.1.exe
	- https://download.qt.io/archive/qt/5.5/5.5.1/qt-opensource-windows-x86-msvc2013_64-5.5.1.exe
- VC12 (Visual Studio 2013)

### Linux only

Install dependencies

- `$> sudo apt-get install mesa-common-dev`
- `$> sudo apt-get install astyle`

Visual Studio Code Extensions

- Astyle
- C/C++
- CMake

## Checkout steps

```bash
$> cd $SOURCES
$> git clone https://github.com/mfreiholz/ocs-vendor-runtimes.git
$> git clone https://github.com/mfreiholz/ocs.git
$> cd ocs
$> git submodule init
$> git submodule update
```

## Compile and build steps

Since OCS requires Qt you have to define where it can be found.
You have to do this by setting the following environment variables:

- `OCS_QTDIR_X86_32`
- `OCS_QTDIR_X86_64`

And then simply run:

### Windows

- `$> build-win<platform>-cmake.bat`
- `$> build-win<platform>-vs2015.bat`

### Linux

- `./build-linux.bash`

## Code style

I began to use AStyle to strictly format source code based on
*astyle-cpp-code-style.cfg* configuration.
