cmake_minimum_required(VERSION 3.0)
project(source)

option(IncludeServerPrograms "IncludeServerPrograms" ON)
option(IncludeClientPrograms "IncludeClientPrograms" ON)

cmake_policy(SET CMP0020 NEW)

include(cmake/compiler.cmake)
include(cmake/thirdparty.cmake)
include(cmake/thirdparty-external.cmake)
include(cmake/project-macro.cmake)

# Generate global version header file.
set(VERSION_MAJOR "0" CACHE INTERNAL "Major software version" FORCE)
set(VERSION_MINOR "14" CACHE INTERNAL "Minor software version" FORCE)
set(VERSION_PATCH "0" CACHE INTERNAL "Patch software version" FORCE)
set(VERSION_EXTRA "" CACHE INTERNAL "Additional software version information, e.g.: release, beta, alpha, dev" FORCE)
FILE(WRITE $ENV{OCS_BUILD_DIR_PATH}/project-defines.h
  "// Generated project version header.
\#ifndef VERSION_H_IN
\#define VERSION_H_IN

\#define IFVS_SOFTWARE_VERSION \"${VERSION_MAJOR}.${VERSION_MINOR}\"
\#define IFVS_SOFTWARE_VERSION_POSTFIX \"${VERSION_EXTRA}\"

\#endif
  "
)
include_directories($ENV{OCS_BUILD_DIR_PATH})

# Libraries with STL dependency.
add_subdirectory(projects/libbase)
add_subdirectory(projects/libcorprotocol)
add_subdirectory(projects/libdaemon)
add_subdirectory(projects/libmediaprotocol)

# Libraries with Qt dependency.
add_subdirectory(projects/libqtcorprotocol)
add_subdirectory(projects/libqtasync)
add_subdirectory(projects/libqtteamspeak3)

# Library for all apps.
add_subdirectory(projects/libapp)

# Server binaries.
if(IncludeServerPrograms)
	add_subdirectory(projects/videoserver)
endif(IncludeServerPrograms)

# Client binaries.
if(IncludeClientPrograms)
	add_subdirectory(projects/libclient)
	add_subdirectory(projects/videoclient)
	add_subdirectory(projects/ts3plugin)
	add_subdirectory(projects/app-qml-client)
endif(IncludeClientPrograms)
add_subdirectory(projects/testapp)
