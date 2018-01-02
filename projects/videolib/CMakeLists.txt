cmake_minimum_required(VERSION 3.8)
project(videolib)

### Qt

cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

### Sources

file(GLOB_RECURSE headers *.h)
file(GLOB_RECURSE sources *.cpp)
file(GLOB_RECURSE qtforms *.ui)
#set(qtres res/app.qrc)

source_group(
	TREE ${CMAKE_CURRENT_SOURCE_DIR}
	PREFIX src
	FILES ${headers} ${sources}
)

### Binaries

add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
	${qtforms}
	${qtres}
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Network
	PUBLIC libbase
)
