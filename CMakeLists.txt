cmake_minimum_required(VERSION 2.8)
project(source)

set(CMAKE_CXX_FLAGS "-std=c++11")

# Third party projects.
add_subdirectory(thirdparty/humblelogging)
set(humblelogging_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/humblelogging/include)
#set(humblelogging_LIBRARIES humblelogging)

# Third party (external projects)
#include(CMakeThirdParty.txt)

# Local projects.
add_subdirectory(projects/corlib) # TCP protocol lib
add_subdirectory(projects/medlib) # UDP protocol lib
add_subdirectory(projects/testexe)

# Local Qt based projects.
if(QTDIR)
    message("Qt found")
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
endif()
add_subdirectory(projects/qcorlib)
add_subdirectory(projects/qswserverlib)
add_subdirectory(projects/qswserverapp)
add_subdirectory(projects/qtestexe)
add_subdirectory(projects/qts3videoclient)
add_subdirectory(projects/qts3videoserver)
