
# Architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x86-64")
	set(ARCH_BIT "64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x86-32")
	set(ARCH_BIT "32")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

# Windows
add_definitions(-DNOMINMAX) # Required for bug in QDateTime header (min())

# Linux
if(CMAKE_COMPILER_IS_GNUCC)
	# GCC 4.8.4 C++11
	#set(CMAKE_CXX_FLAGS "-std=c++11")
	# GCC 4.8.4 C++14
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	# GCC 5.2.X C++14
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

	# Strict flags
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif(CMAKE_COMPILER_IS_GNUCC)
