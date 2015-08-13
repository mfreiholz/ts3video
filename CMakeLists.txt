cmake_minimum_required(VERSION 2.8)
project(source)
cmake_policy(SET CMP0020 NEW)

include(cmake/compiler.cmake)
include(cmake/thirdparty.cmake)
include(cmake/thirdparty-external.cmake)

# Generate global version header file.
set(VERSION_MAJOR "0" CACHE INTERNAL "Major software version" FORCE)
set(VERSION_MINOR "4" CACHE INTERNAL "Minor software version" FORCE)
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

# Projects to build.
add_subdirectory(projects/corlib)
add_subdirectory(projects/medlib)
add_subdirectory(projects/humblesrvproc)
add_subdirectory(projects/qtasync)
add_subdirectory(projects/qcorlib)
add_subdirectory(projects/qts3serverquerylib)
add_subdirectory(projects/videolib)
add_subdirectory(projects/videoserver)
if(WIN32)
  add_subdirectory(projects/videoclient)
  add_subdirectory(projects/ts3plugin)
endif(WIN32)
