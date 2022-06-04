cmake_minimum_required(VERSION 3.8)
project(testapp)

### Qt

cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5OpenGL REQUIRED)

### Sources

file(GLOB_RECURSE headers ./src/*.h)
file(GLOB_RECURSE sources ./src/*.cpp)
file(GLOB_RECURSE qtforms ./src/*.ui)
#set(qtres res/res.qrc)

source_group(
	TREE ${CMAKE_CURRENT_SOURCE_DIR}
	FILES ${headers} ${sources}
)

### Binaries

set(GUI_TYPE "")
if (WIN32)
	set(GUI_TYPE WIN32)
endif()

add_executable(
	${PROJECT_NAME}
	${GUI_TYPE}
	${headers}
	${sources}
	${qtforms}
	${qtres}
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE -DOCS_INCLUDE_OPENGL
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${vpx_INCLUDE_DIRS}
	PRIVATE ${humblelogging_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE ${vpx_LIBRARIES}
	PRIVATE ${humblelogging_LIBRARIES}
	PRIVATE Qt5::Core
	PRIVATE Qt5::Gui
	PRIVATE Qt5::Widgets
	PRIVATE Qt5::Network
	PRIVATE Qt5::Multimedia
	PRIVATE Qt5::MultimediaWidgets
	PRIVATE Qt5::Concurrent
	PRIVATE libqtasync
	PRIVATE libqtcorprotocol
	PRIVATE libbase
	PRIVATE libmediaprotocol
	PRIVATE libapp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::OpenGL)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE opengl32) # TODO This shouldn't be here!
else()
	if(IncludeOpenGLSupport)
		target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/nvidia-375/libGL.so)
#		target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/mesa/libGL.so.1)
	endif()
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()
