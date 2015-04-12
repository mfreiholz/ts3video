cmake_minimum_required(VERSION 2.8)
project(qts3videoserver)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED) # Due to qts3videocorelib
find_package(Qt5WebSockets REQUIRED)

# Sources
set(headers
)

set(qheaders
  src/ts3videoserver.h
  src/clientconnectionhandler.h
  src/mediasockethandler.h
  src/websocketstatusserver.h
)

set(sources
  src/main.cpp
  src/ts3videoserver.cpp
  src/clientconnectionhandler.cpp
  src/mediasockethandler.cpp
  src/websocketstatusserver.cpp
)

set(qforms
)

set(qresources
  res/res.qrc
)

if(WIN32)
	list(APPEND sources res/app.rc)
endif(WIN32)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DNOMINMAX
)

# Includes
include_directories(
  ${humblelogging_INCLUDE_DIRS}
  ../humblesrvproc/include
  ../medlib/include
  ../qcorlib/src
  ../qts3videocorelib/src
)

# Target prerequisites.
if(WIN32)
  set(win32_gui_type WIN32)
endif(WIN32)

# Target
add_executable(
  ${PROJECT_NAME}
  ${win32_gui_type}
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
  Qt5::Gui
  Qt5::WebSockets
  humblelogging
  humblesrvproc
  qcorlib
  qts3videocorelib
  medlib
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "videoserver"
)

humblelogging_POSTBUILD(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
