
# Qt5
find_package(Qt5Core REQUIRED)

# Sources
set(headers
  src/plugin.h
)

set(sources
  src/plugin.c
  src/impl.cpp
)

# Defines
add_definitions(
)

# Includes
include_directories(
  ${ts3pluginsdk_INCLUDE_DIRS}
)

# Target
add_library(
  qts3videoclientplugin
  SHARED
  ${headers}
  ${sources}
)

target_link_libraries(
  qts3videoclientplugin
  Qt5::Core
)