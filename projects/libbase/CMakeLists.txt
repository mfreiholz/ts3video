cmake_minimum_required(VERSION 2.8)
project(libbase)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

set(headers
	defines.h
	exception.h
)

set(sources
	exception.cpp
)

add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
)