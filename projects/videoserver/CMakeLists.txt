cmake_minimum_required(VERSION 2.8)
project(videoserver)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED) # Due to videolib
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Concurrent REQUIRED)

# Sources
set(headers
  src/servercliententity.h
  src/serverchannelentity.h
  src/virtualserveroptions.h
  src/action/actionbase.h
)

set(qheaders
  src/virtualserver.h
  src/clientconnectionhandler.h
  src/mediasockethandler.h
  src/websocketstatusserver.h
)

set(sources
  src/main.cpp
  src/virtualserveroptions.cpp
  src/virtualserver.cpp
  src/clientconnectionhandler.cpp
  src/mediasockethandler.cpp
  src/websocketstatusserver.cpp
  src/servercliententity.cpp
  src/serverchannelentity.cpp
  src/action/actionbase.cpp
)

set(qresources
#  res/res.qrc
)

if(WIN32)
	list(APPEND sources res/app.rc)
endif(WIN32)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DNOMINMAX
)

# Includes
include_directories(
  ${humblelogging_INCLUDE_DIRS}
  ../humblesrvproc/include
  ../qtasync/src
  ../medlib/include
  ../qcorlib/src
  ../qts3serverquerylib/src
  ../videolib/src
)

# Target prerequisites.
if(WIN32)
  #set(win32_gui_type WIN32)
endif(WIN32)

# Target
add_executable(
  ${PROJECT_NAME}
  ${win32_gui_type}
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  ${PROJECT_NAME}
  Qt5::Core
  Qt5::Network
  Qt5::Gui
  Qt5::WebSockets
  Qt5::Concurrent
  ${humblelogging_LIBRARIES}
  humblesrvproc
  qtasync
  medlib
  qcorlib
  qts3serverquerylib
  videolib
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "videoserver"
)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/res/default.ini ${PROJECT_BINARY_DIR}/$<CONFIGURATION>
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/res/logging.conf ${PROJECT_BINARY_DIR}/$<CONFIGURATION>
)

#######################################################################
# Install
#######################################################################

if(WIN32)
  set(INST_BINARY_DIR "server/bin/win-${ARCH}")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCC)
  set(INST_BINARY_DIR "server/bin/linux-${ARCH}")
endif(CMAKE_COMPILER_IS_GNUCC)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INST_BINARY_DIR})
install(FILES res/default.ini DESTINATION ${INST_BINARY_DIR})
install(FILES res/logging.conf DESTINATION ${INST_BINARY_DIR})
