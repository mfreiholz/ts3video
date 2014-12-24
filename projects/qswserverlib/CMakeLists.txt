
# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5WebSockets REQUIRED)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

# Sources
set(headers
  include/servercore/api.h
  include/servercore/includes.h
)

set(qheaders
  #src/network/master/masterserver.h
  #src/network/master/masterserver_p.h
  #src/network/master/mastermedianodeconnectionhandler.h
  #src/network/master/mastermedianodeconnectionhandler_p.h
  #src/network/master/masterclientnodeconnectionhandler.h
  #src/network/master/masterclientnodeconnectionhandler_p.h
  #src/network/master/masterctrlwsconnectionhandler.h
  #src/network/master/masterctrlwsconnectionhandler_p.h
  #src/network/streaming/mediastreamingserver.h
  #src/network/streaming/mediastreamingserver_p.h
  #src/network/streaming/mediastreamingsockethandler.h
  #src/network/streaming/mediastreamingsockethandler_p.h
  #src/network/streaming/mediastreamingmasterconnectionhandler.h
  #src/network/streaming/mediastreamingmasterconnectionhandler_p.h
  #src/network/streaming/mediaserializables.h
  src/network/control/module/controlmodule.h
  src/network/control/controlapi.h
  src/network/control/controlserver.h
  src/network/control/controlserver_p.h
  src/network/control/controltomasterconnectionhandler.h
  src/network/control/controltomasterconnectionhandler_p.h
  src/network/control/controlconnectionhandler.h
  src/network/control/controlconnectionhandler_p.h
  #src/network/tcpprotocol.h
  #src/network/udpprotocol.h
)

set(sources
  #src/network/master/masterserver.cpp
  #src/network/master/mastermedianodeconnectionhandler.cpp
  #src/network/master/masterclientnodeconnectionhandler.cpp
  #src/network/master/masterctrlwsconnectionhandler.cpp
  #src/network/streaming/mediastreamingserver.cpp
  #src/network/streaming/mediastreamingsockethandler.cpp
  #src/network/streaming/mediastreamingmasterconnectionhandler.cpp
  #src/network/streaming/mediaserializables.cpp
  src/network/control/module/controlmodule.cpp
  src/network/control/controlserver.cpp
  src/network/control/controltomasterconnectionhandler.cpp
  src/network/control/controlconnectionhandler.cpp
  #src/network/tcpprotocol.cpp
  #src/network/udpprotocol.cpp
)

set(qforms
)

set(qresources
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DUNICODE
  -D_UNICODE
  -DNOMINMAX
  -DSERVERCORE_EXPORT
)

# Includes
include_directories(
  src
  include
  ${HumbleLogging_INCLUDE_DIRS}
  ../qcorlib/src
  ../qcorlib/include
)

# Target
add_library(
  qswserverlib
  STATIC
  ${sources}
  ${headers}
  ${qheaders}
  ${qheaders_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qswserverlib
  Qt5::Core
  Qt5::Network
  Qt5::Sql
  Qt5::WebSockets
  HumbleLogging
  qcorlib
)

if(WIN32)
  target_link_libraries(
    qswserverlib
    Ws2_32.dll
  )
endif()

add_dependencies(
  qswserverlib
  HumbleLogging
)