
# Sources
set(headers
  include/corprotocol.h
  include/udpprotocol.h
)

set(sources
  src/corprotocol.cpp
  src/udpprotocol.cpp
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