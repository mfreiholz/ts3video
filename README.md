
# Build
Build has been tested on
- Windows 8.1


## Requirements
- CMake (build system)
- Qt >=5.3 (lower versions not tested yet)
- VC12 (Visual Studio 2013)
- Optional: Qt Addin for Visual Studio (http://www.qt.io/download-open-source/)


## Checkout steps
```
$> git clone [URI] [CHECKOUT_DIR]
$> cd [CHECKOUT_DIR]
$> git submodule init
$> git submodule update
```


## Build steps
```
$> cd [CHECKOUT_DIR]
$> mkdir build
$> cd build
$> cmake -G "Visual Studio 12 Win64" ..
```


# Useful commands
Remove all non-tracked + ignored files (append `-n` for a dry-run)
```
git clean -x -d -f
```
