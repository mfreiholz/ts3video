cmake_minimum_required(VERSION 2.8)
project(qcorlib)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
	include/${PROJECT_NAME}/qcorframe.h
)

set(qheaders
	include/${PROJECT_NAME}/qcorconnection.h
	include/${PROJECT_NAME}/qcorserver.h
	include/${PROJECT_NAME}/qcorreply.h
	src/qcorconnection_p.h
	src/qcorserver_p.h
	
)

set(sources
  src/qcorconnection.cpp
  src/qcorframe.cpp
  src/qcorserver.cpp
  src/qcorreply.cpp
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
  ../corlib/include
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
  corlib
  Qt5::Core
  Qt5::Network
)
