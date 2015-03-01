cmake_minimum_required(VERSION 2.8)
project(source)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install")

include(cmake/compiler.cmake)
include(cmake/thirdparty.cmake)

# Local projects.
add_subdirectory(projects/corlib)
add_subdirectory(projects/medlib)
add_subdirectory(projects/qcorlib)
add_subdirectory(projects/qts3videocorelib)
add_subdirectory(projects/qts3videoserver)
add_subdirectory(projects/qts3videoclient)

# TS3 Client Plugin
add_subdirectory(projects/qts3videoclientplugin)

# Obsolete
#add_subdirectory(projects/qswserverlib)
#add_subdirectory(projects/qswserverapp)

# Tests
#add_subdirectory(projects/testexe)
#add_subdirectory(projects/qtestexe)