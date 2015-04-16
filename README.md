
# Build
Build has been tested on
- Windows 8.1


## Requirements
- CMake (build system)
- Qt 5.4
- VC12 (Visual Studio 2013)
- Optional: Qt Add-in for Visual Studio (http://www.qt.io/download-open-source/)


## Checkout steps
```
$> git clone git@git.insanefactory.com:/opt/git/ts3video.git .
$> cd ts3video
$> git submodule init
$> git submodule update
```


## Compile and build steps
```
$> mkdir build
$> cd build
$> cmake -G "Visual Studio 12 Win64" ..
```


# Useful commands
Remove all non-tracked + ignored files (append `-n` for a dry-run)
```
git clean -x -d -f
```
