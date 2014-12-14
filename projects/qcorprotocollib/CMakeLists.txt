
# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

# Sources
set(headers

)

set(qheaders
  src/qcorconnection.h
  src/qcorframe.h
  src/qcorserver.h
)

set(sources
  src/qcorconnection.cpp
  src/qcorframe.cpp
  src/qcorserver.cpp
)

set (qforms

)

set (qresources

)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
#qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})

# Defines
add_definitions(
)

# Includes
include_directories(
  include
  src
  ../networkcore/include
)

# Target
add_library(
  qcorprotocollib
  STATIC
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qcorprotocollib
  networkcore
  Qt5::Core
  Qt5::Network
)