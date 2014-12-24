
# Sources
set(headers
  include/medprotocol.h
)

set(sources
  src/medprotocol.cpp
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
  medlib
  STATIC
  ${headers}
  ${sources}
)

if(WIN32)
  target_link_libraries(
    medlib
    Ws2_32.dll
  )
endif()