# Create a new release

## All platforms

- Check version configuration in
	- *CMakeLists.txt*
	- *projects/videolib/ts3video.h*
	- *innosetup/all.iss*

## Windows

- Build *Release* version of 32-bit
	- *build-win32-cmake.bat*
	- *build-win32-vs2015.bat* - Do not forget to run the *INSTALL* project inside Visual Studio
	- *build-win32-release.bat*
- Build *Release* version of 64-bit
	- *build-win64-cmake.bat*
	- *build-win64-vs2015.bat* - Do not forget to run the *INSTALL* project inside Visual Studio
	- *build-win64-release.bat*

Now check your `%OCS_RELEASE_DIR_PATH%` (RELEASE_DIR). It will contain installers for the release.

## Linux

Not yet documented.