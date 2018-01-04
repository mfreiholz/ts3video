cmake_minimum_required(VERSION 3.8)
project(libbase)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

### Sources

file(GLOB_RECURSE headers ${PROJECT_NAME}/*.h)
file(GLOB_RECURSE sources ${PROJECT_NAME}/*.cpp)

source_group(
	TREE ${CMAKE_CURRENT_SOURCE_DIR}
	FILES ${headers} ${sources}
)

### Binaries

add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
