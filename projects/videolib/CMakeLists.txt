cmake_minimum_required(VERSION 2.8)
project(videolib)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)

# Sources
set(headers
  src/ts3video.h
  src/cliententity.h
  src/channelentity.h
  src/jsonprotocolhelper.h
  src/elws.h
  src/yuvframe.h
  src/vp8frame.h
  src/imageutil.h
  src/timeutil.h
  src/networkusageentity.h
)

set(qheaders
)

set(sources
  src/cliententity.cpp
  src/channelentity.cpp
  src/jsonprotocolhelper.cpp
  src/elws.cpp
  src/yuvframe.cpp
  src/vp8frame.cpp
  src/imageutil.cpp
  src/timeutil.cpp
  src/networkusageentity.cpp
)

set(qforms
)

set(qresources
  #res/res.qrc
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
#qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
)

# Includes
include_directories(
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
  Qt5::Gui
  Qt5::Network
)