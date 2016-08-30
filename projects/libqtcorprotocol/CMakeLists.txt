cmake_minimum_required(VERSION 2.8)
project(libqtcorprotocol)
cmake_policy(SET CMP0020 NEW)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

set(headers
	qcorframe.h
)

set(qheaders
	qcorconnection.h
	qcorserver.h
	qcorreply.h
	qcorconnection_p.h
	qcorserver_p.h
)

set(sources
	qcorconnection.cpp
	qcorframe.cpp
	qcorserver.cpp
	qcorreply.cpp
)

set (qforms
)

set (qresources
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

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
  libcorprotocol
)