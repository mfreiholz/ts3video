cmake_minimum_required(VERSION 2.8)
project(qts3serverquerylib)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
)

set(qheaders
  src/ts3serverbridge.h
  src/ts3serverquery.h
  src/ts3serverqueryconnection.h
  src/ts3util.h
)

set(sources
  src/ts3serverbridge.cpp
  src/ts3serverquery.cpp
  src/ts3serverqueryconnection.cpp
  src/ts3util.cpp
)

set (qforms
)

set (qresources
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
)

# Includes
include_directories(
  include
  src
  ${humblelogging_INCLUDE_DIRS}
)

# Target
add_library(
  ${PROJECT_NAME}
  STATIC
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  ${PROJECT_NAME}
  Qt5::Core
  Qt5::Network
  ${humblelogging_LIBRARIES}
)
