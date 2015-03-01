cmake_minimum_required(VERSION 2.8)
project(qts3videoclientplugin)
cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)

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
  qts3videoclientplugin
  SHARED
  ${headers}
  ${sources}
)

# Install
install(DIRECTORY package DESTINATION ts3plugin)
install(TARGETS qts3videoclientplugin DESTINATION ts3plugin/package/plugins)