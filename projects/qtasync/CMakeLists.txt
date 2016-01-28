cmake_minimum_required(VERSION 2.8)
project(qtasync)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
	include/${PROJECT_NAME}/qtasync.h
)

set(qheaders
)

set(sources
	src/qtasync.cpp
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
  Qt5::Concurrent
)
