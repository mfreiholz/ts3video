
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
  src/mytestobject.h
  src/qtestclient.h
)

set(sources
  src/mytestobject.cpp
  src/qtestclient.cpp
  src/main.cpp
)

set (qforms

)

set (qresources

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
  include
  src
  ../qcorprotocollib/src
)

# Target
add_executable(
  qtestexe
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qtestexe
  qcorprotocollib
  Qt5::Core
  Qt5::Network
)