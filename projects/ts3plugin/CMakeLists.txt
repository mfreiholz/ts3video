cmake_minimum_required(VERSION 2.8)
project(ts3plugin)
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
  -D_CRT_SECURE_NO_WARNINGS
)

# Includes
include_directories(
  ${ts3pluginsdk_INCLUDE_DIRS}
  ../videolib/src             # Used for ts3video.h (Version information, ...)
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
# -------
set(TS3_PLUGIN_PACKAGE_DIR "${CMAKE_INSTALL_PREFIX}/ts3_plugin")

# Copy plugin dll.
install(TARGETS ${PROJECT_NAME} DESTINATION ts3_plugin/plugins)

# Generate "package.ini" in "ts3_plugin"
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DVERSION_MAJOR=${VERSION_MAJOR} -DVERSION_MINOR=${VERSION_MINOR} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -P \"${CMAKE_CURRENT_LIST_DIR}/cmake/generate-package-ini.cmake\")")

# Copy "videoclient" into plugin package directory.
#install(SCRIPT cmake/copy-videoclient-into-package.cmake)
