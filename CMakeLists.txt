cmake_minimum_required(VERSION 3.0)
project(source)

cmake_policy(SET CMP0020 NEW)

include(cmake/compiler.cmake)
include(cmake/thirdparty.cmake)
include(cmake/thirdparty-external.cmake)

# Generate global version header file.
set(VERSION_MAJOR "0" CACHE INTERNAL "Major software version" FORCE)
set(VERSION_MINOR "12" CACHE INTERNAL "Minor software version" FORCE)
set(VERSION_PATCH "0" CACHE INTERNAL "Patch software version" FORCE)
set(VERSION_EXTRA "" CACHE INTERNAL "Additional software version information, e.g.: release, beta, alpha, dev" FORCE)
FILE(WRITE projects/project-defines.h
  "// Generated project version header.
\#ifndef VERSION_H_IN
\#define VERSION_H_IN

\#define IFVS_SOFTWARE_VERSION \"${VERSION_MAJOR}.${VERSION_MINOR}\"
\#define IFVS_SOFTWARE_VERSION_POSTFIX \"${VERSION_EXTRA}\"

\#endif
  "
)
include_directories(projects)

# project independend libs (+stl)
add_subdirectory(projects/libcorprotocol)
add_subdirectory(projects/libdaemon)

# project independend libs (+qt)
add_subdirectory(projects/libqtcorprotocol)
add_subdirectory(projects/libqtasync)

# project specific libs (+stl)
add_subdirectory(projects/libbase)
add_subdirectory(projects/libmediaprotocol)

# project specific libs (+qt)
add_subdirectory(projects/libqtteamspeak3)

# binaries
add_subdirectory(projects/videolib)
add_subdirectory(projects/videoserver)
add_subdirectory(projects/videoclient)
if(WIN32)
	add_subdirectory(projects/ts3plugin)
endif(WIN32)
