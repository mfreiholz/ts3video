
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
)

set(qheaders
)

set(sources
  src/main.cpp
)

set(qresources
  res/res.qrc
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DNOMINMAX
)

# Includes
include_directories(
  src
  ${humblelogging_INCLUDE_DIRS}
  ../qswserverlib/include
)

# Target
add_executable(
  qswserverapp
  ${headers}
  ${qheaders}
  ${sources}
  ${qheaders_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qswserverapp
  Qt5::Core
  Qt5::Network
  Qt5::Sql
  Qt5::WebSockets
  qswserverlib
)

if(WIN32)
  target_link_libraries(
    qswserverapp
    Ws2_32.dll
  )
endif()

add_dependencies(
  qswserverapp
  humblelogging
  qswserverlib
)