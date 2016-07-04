cmake_minimum_required(VERSION 2.8)
project(libts3)

cmake_policy(SET CMP0020 NEW)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

set(headers
	ts3serverquery.h
	ts3util.h
)

set(qheaders
	ts3queryconsolesocket.h
)

set(sources
	ts3queryconsolesocket.cpp
	ts3serverquery.cpp
	ts3util.cpp
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
source_group(QtWrap FILES ${qheaders_wrap})

include_directories(
	${humblelogging_INCLUDE_DIRS}
)

add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
	${qheaders}
	${qheaders_wrap}
)

target_link_libraries(
  ${PROJECT_NAME}
  Qt5::Core
  Qt5::Network
  ${humblelogging_LIBRARIES}
)