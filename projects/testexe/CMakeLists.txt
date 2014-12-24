
# Sources
set(headers
)

set(sources
  src/main.cpp
)

# Defines
add_definitions(
)

# Includes
include_directories(
  include
  src
  ../corlib/include
  ../medlib/include
)

# Target
add_executable(
  testexe
  ${headers}
  ${sources}
)

target_link_libraries(
  testexe
  corlib
  medlib
)