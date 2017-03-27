cmake_minimum_required(VERSION 2.8)
project(videolib)
cmake_policy(SET CMP0020 NEW)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

set(headers
	ts3video.h
	cliententity.h
	channelentity.h
	virtualserverconfigentity.h
	jsonprotocolhelper.h
	elws.h
	yuvframe.h
	vp8frame.h
	pcmframe.h
	opusframe.h
	imageutil.h
	timeutil.h
	networkusageentity.h
	license.h
)

set(qheaders
)

set(sources
	cliententity.cpp
	channelentity.cpp
	virtualserverconfigentity.cpp
	jsonprotocolhelper.cpp
	elws.cpp
	pcmframe.cpp
	opusframe.cpp
	yuvframe.cpp
	vp8frame.cpp
	imageutil.cpp
	timeutil.cpp
	networkusageentity.cpp
	license.cpp
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
source_group(QtWrap FILES ${qheaders_wrap})

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
  Qt5::Gui
  Qt5::Network
  libbase
)