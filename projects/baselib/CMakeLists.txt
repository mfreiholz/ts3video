cmake_minimum_required(VERSION 2.8)
project(baselib)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
	include/baselib/defines.h
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
)

# Target
add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
)