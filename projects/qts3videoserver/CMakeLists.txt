
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
  #res/res.qrc
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
#qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DNOMINMAX
)

# Includes
include_directories(
  ${humblelogging_INCLUDE_DIRS}
  ../medlib/include
  ../qcorlib/src
  ../qts3videocorelib/src
)

# Target
add_executable(
  qts3videoserver
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qts3videoserver
  Qt5::Core
  Qt5::Network
  Qt5::Gui
  Qt5::WebSockets
  humblelogging
  qcorlib
  qts3videocorelib
  medlib
)

humblelogging_POSTBUILD(qts3videoserver)

install(TARGETS qts3videoserver DESTINATION bin)