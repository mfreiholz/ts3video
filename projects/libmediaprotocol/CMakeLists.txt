cmake_minimum_required(VERSION 2.8)
project(libmediaprotocol)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

set(headers
	protocol.h
)

set(sources
	protocol.cpp
)

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