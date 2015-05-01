
# Architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x86-64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x86-32")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

# Windows
add_definitions(-DNOMINMAX) # Required for bug in QDateTime header (min())

# Linux
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "-std=c++11")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif(CMAKE_COMPILER_IS_GNUCC)
