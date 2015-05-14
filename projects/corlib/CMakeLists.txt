cmake_minimum_required(VERSION 2.8)
project(corlib)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
  include/corprotocol.h
  include/corparser.h
)

set(sources
  src/corparser.c
)

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
  corlib
  STATIC
  ${headers}
  ${sources}
)

if(WIN32)
  target_link_libraries(
    corlib
    Ws2_32.dll
  )
endif()
