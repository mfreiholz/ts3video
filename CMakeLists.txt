cmake_minimum_required(VERSION 2.8)
project(source)

# Architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x86")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

# Global compiler flags.
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCC)

# Third party (HumbleLogging)
add_subdirectory(thirdparty/humblelogging)
set(humblelogging_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/humblelogging/include)
#set(humblelogging_LIBRARIES humblelogging)
macro(humblelogging_POSTBUILD target_)
  add_custom_command(
    TARGET ${target_} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${PROJECT_SOURCE_DIR}/build/thirdparty/humblelogging/bin/$<CONFIGURATION>/humblelogging.dll"
      $<TARGET_FILE_DIR:${target_}>/humblelogging.dll
  )
endmacro(humblelogging_POSTBUILD)

# Third party (VPX)
set(vpx_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/vpx/include
)
set(vpx_LIBRARIES
  optimized "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-release/vpxmd.lib"
  debug "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-debug/vpxmdd.lib"
)

# Third party (ts3pluginsdk)
set(ts3pluginsdk_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/ts3pluginsdk/include/ts3
)

# Third party (external projects)
#include(CMakeThirdParty.txt)

# Local projects.
add_subdirectory(projects/corlib) # TCP protocol lib
add_subdirectory(projects/medlib) # UDP protocol lib

# Local Qt based projects.
#if(QTDIR)
#  message("Qt found")
#  list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
#endif()

cmake_policy(SET CMP0020 NEW)

add_subdirectory(projects/qcorlib)
add_subdirectory(projects/qts3videocorelib)
add_subdirectory(projects/qts3videoserver)
add_subdirectory(projects/qts3videoclient)

# TS3 Client Plugin
add_subdirectory(projects/qts3videoclientplugin)

# Obsolete
#add_subdirectory(projects/qswserverlib)
#add_subdirectory(projects/qswserverapp)

# Tests
#add_subdirectory(projects/testexe)
#add_subdirectory(projects/qtestexe)