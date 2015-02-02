cmake_minimum_required(VERSION 2.8)
project(source)

set(CMAKE_CXX_FLAGS "-std=c++11")

# Third party projects.
add_subdirectory(thirdparty/humblelogging)
set(humblelogging_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/humblelogging/include)
#set(humblelogging_LIBRARIES humblelogging)

# Third party (VPX)
set(vpx_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/vpx/include
)
set(vpx_LIBRARIES
  optimized "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-x86-vc11-release/vpxmd.lib"
  debug "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-x86-vc11-debug/vpxmdd.lib"
)

# Third party (external projects)
#include(CMakeThirdParty.txt)

# Local projects.
add_subdirectory(projects/corlib) # TCP protocol lib
add_subdirectory(projects/medlib) # UDP protocol lib
#add_subdirectory(projects/testexe)

# Local Qt based projects.
if(QTDIR)
    message("Qt found")
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
endif()
cmake_policy(SET CMP0020 NEW)

add_subdirectory(projects/qcorlib)
#add_subdirectory(projects/qswserverlib)
#add_subdirectory(projects/qswserverapp)
#add_subdirectory(projects/qtestexe)
add_subdirectory(projects/qts3videoclient)
add_subdirectory(projects/qts3videoserver)
add_subdirectory(projects/qts3videocorelib)