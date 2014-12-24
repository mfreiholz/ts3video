# Build

Build has been tested on
- Windows 8.1
- Ubuntu 14.04

## Requirements

- CMake (build system)
- Qt >=5.3 (lower versions not tested yet)
- Optional: Qt Addin for Visual Studio 2012 (http://www.qt.io/download-open-source/)

## Steps
```
$> cd $PROJECT_DIRECTORY
$> mkdir build
$> cd build
$> cmake ..
```


# Useful commands

Remove all non-tracked + ignored files (append `-n` for a dry-run)
```
git clean -x -d -f
```

