
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
  src/main.cpp
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
)