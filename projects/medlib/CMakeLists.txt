cmake_minimum_required(VERSION 2.8)
project(medlib)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Sources
set(headers
	include/medlib/protocol.h
)

set(sources
	src/protocol.cpp
)

# Defines
add_definitions(
)

# Includes
include_directories(
	include
	src
	../baselib/include
)

# Target
add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
)

if(WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		Ws2_32.dll
	)
endif()