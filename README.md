![Logo](https://raw.githubusercontent.com/mfreiholz/ocs/master/projects/videoclient/res/logo-48x48.png)

# Build
Build has been tested on
- Windows 8.1
- Ubuntu 15.04


## Requirements
- CMake 2.8.11
- Qt 5.4 with OpenGL
- VC12 (Visual Studio 2013)
- Optional: Qt Add-in for Visual Studio (http://www.qt.io/download-open-source/)
- Linux: sudo apt-get install mesa-common-dev


## Checkout steps
```
$> git clone https://github.com/mfreiholz/ocs.git
$> cd ocs
$> git submodule init
$> git submodule update
```


## Compile and build steps
```
$> mkdir build
$> cd build
$> cmake -DCMAKE_INSTALL_PREFIX=install -G "Visual Studio 12 Win64" ..
```


# Useful commands
Remove all non-tracked + ignored files (append `-n` for a dry-run)
```
git clean -x -d -f
```

# Related repositories
- https://github.com/mfreiholz/ts3video-homepage
- https://github.com/mfreiholz/ocs-deploy
- https://github.com/mfreiholz/ocs-ts3clientpluginsdk
- https://github.com/mfreiholz/ocs-vpx
