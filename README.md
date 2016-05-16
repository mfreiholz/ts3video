![Logo](https://raw.githubusercontent.com/mfreiholz/ocs/master/projects/videoclient/res/logo-48x48.png)


# Build

Build has been tested on
- Windows 8.1
- Ubuntu 14.04
- Ubuntu 15.10


## Requirements

- CMake >= 3.0.0
- Qt 5.4.1 build with OpenGL
- VC12 (Visual Studio 2013)

__Linux only__
- `$> sudo apt-get install mesa-common-dev`


## Checkout steps

```
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

__Windows__
- `$> cmake-win-<platform>.bat`
- `$> start-vs2015-<platform>.bat`

__Linux__
- `./build-linux.bash`


## Code style

I began to use AStyle to strictly format source code based on
*astyle-cpp-code-style.cfg* configuration.

