include(ExternalProject)

#######################################################################
# HumbleLogging
#######################################################################

ExternalProject_Add(
  humblelogging
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/humblelogging"
  CMAKE_ARGS -DBuildShared=OFF -DBuildExamples=OFF -DBuildSystemNative=ON
  INSTALL_COMMAND ""
)

# Includes
set(humblelogging_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdparty/humblelogging/include")

# Link libraries
if(WIN32)
  set(humblelogging_LIBRARIES
    optimized "${CMAKE_BINARY_DIR}/humblelogging-prefix/src/humblelogging-build/Release/humblelogging.lib"
    debug "${CMAKE_BINARY_DIR}/humblelogging-prefix/src/humblelogging-build/Debug/humblelogging.lib"
  )
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCC)
  set(humblelogging_LIBRARIES
    "${CMAKE_BINARY_DIR}/humblelogging-prefix/src/humblelogging-build/libhumblelogging.a"
  )
endif(CMAKE_COMPILER_IS_GNUCC)

