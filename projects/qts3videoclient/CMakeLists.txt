
# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

# Sources
set(headers
)

set(qheaders
  src/clientvideowidget.h
  src/gridviewwidgetarranger.h
  src/videocollectionwidget.h
  src/ts3videoclient.h
  src/ts3videoclient_p.h
)

set(sources
  src/main.cpp
  src/clientvideowidget.cpp
  src/gridviewwidgetarranger.cpp
  src/videocollectionwidget.cpp
  src/ts3videoclient.cpp
)

set(qforms
  src/clientvideowidget.ui
)

set(qresources
  res/res.qrc
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
)

# Includes
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ../medlib/include
  ../qcorlib/src
  ../qts3videocorelib/src
)

# Target
add_executable(
  qts3videoclient
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qts3videoclient
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Network
  qcorlib
  qts3videocorelib
  medlib
)