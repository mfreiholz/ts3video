cmake_minimum_required(VERSION 2.8)
project(humblesrvproc)

# Sources
set(headers
  include/humblesrvproc/api.h
)

set(sources
  src/main.c
)

if(WIN32)
  list(APPEND sources src/impl_windows.c)
else(WIN32)
  list(APPEND sources src/impl_linux.c)
endif(WIN32)

# Defines
add_definitions(
)

# Includes
include_directories(
  include
  src
)

# Target
add_library(
  ${PROJECT_NAME}
  STATIC
  ${headers}
  ${sources}
)
