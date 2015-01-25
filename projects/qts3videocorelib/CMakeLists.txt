
# Qt5
find_package(Qt5Core REQUIRED)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

# Sources
set(headers
  src/cliententity.h
  src/channelentity.h
  src/jsonprotocolhelper.h
)

set(qheaders
)

set(sources
  src/cliententity.cpp
  src/channelentity.cpp
  src/jsonprotocolhelper.cpp
)

set(qforms
)

set(qresources
  res/res.qrc
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
  qts3videocorelib
  STATIC
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qts3videocorelib
  Qt5::Core
)