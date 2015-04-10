cmake_minimum_required(VERSION 2.8)
project(qts3videoclientplugin)
cmake_policy(SET CMP0020 NEW)

# Sources
set(headers
  src/plugin.h
  src/impl.h
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
  ${PROJECT_NAME}
  SHARED
  ${headers}
  ${sources}
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "ts3video"
)

# Install
## Generate "package.ini"
install(SCRIPT cmake/generate-package-ini.cmake)
## Copy plugin dll.
install(TARGETS ${PROJECT_NAME} DESTINATION ts3video/plugins)
## Copy "qts3videoclient" target into plugin's directory.
#install(TARGETS qts3videoclient DESTINATION ts3video/plugins/ts3video)
