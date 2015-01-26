
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
)

set(sources
)

set (qforms
)

set (qresources
)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
)

# Includes
include_directories(
  src
  ../medlib/include
)

# Target
add_library(
  qmedlib
  STATIC
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qmedlib
  medlib
  Qt5::Core
  Qt5::Network
)
