cmake_minimum_required(VERSION 3.8)
project(videoclient)

option(IncludeOpenGLSupport "IncludeOpenGLSupport" OFF)
option(IncludeAudioSupport "IncludeAudioSupport" OFF)

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

if(IncludeOpenGLSupport)
	find_package(Qt5OpenGL REQUIRED)
endif(IncludeOpenGLSupport)

### Sources

file(GLOB_RECURSE headers ./src/*.h)
file(GLOB_RECURSE sources ./src/*.cpp)
file(GLOB_RECURSE qtforms ./src/*.ui)
set(qtres res/res.qrc)

if(WIN32)
	list(APPEND sources res/app.rc)
endif(WIN32)

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

if(IncludeOpenGLSupport)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE -DOCS_INCLUDE_OPENGL
	)
endif(IncludeOpenGLSupport)

if(IncludeAudioSupport)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE -DOCS_INCLUDE_AUDIO
	)
endif(IncludeAudioSupport)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${vpx_INCLUDE_DIRS}
	PRIVATE ${opus_INCLUDE_DIRS}
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

if(IncludeOpenGLSupport)
	target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::OpenGL)
endif()

if(WIN32)
	if(IncludeOpenGLSupport)
		target_link_libraries(${PROJECT_NAME} PRIVATE opengl32) # TODO This shouldn't be here!
	endif()
else()
	if(IncludeOpenGLSupport)
		target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/nvidia-375/libGL.so)
#		target_link_libraries(${PROJECT_NAME} PRIVATE /usr/lib/x86_64-linux-gnu/mesa/libGL.so.1)
	endif()
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

if(IncludeAudioSupport)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${opus_LIBRARIES})
endif()

set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME "videoclient"
)

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/res/default.ini ${PROJECT_BINARY_DIR}/$<CONFIGURATION>
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/res/logging.conf ${PROJECT_BINARY_DIR}/$<CONFIGURATION>
)

#######################################################################
# Install
#######################################################################

install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/client)
install(FILES ${PROJECT_SOURCE_DIR}/res/default.ini DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/client RENAME default.ini.orig)
install(FILES ${PROJECT_SOURCE_DIR}/res/logging.conf DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/client RENAME logging.conf.orig)

#if(WIN32)
#	set(DEPLOY_DIR_PATH $ENV{OCS_INSTALL_DIR_PATH}/client)
#	string(REPLACE "\\" "/" DEPLOY_DIR_PATH ${DEPLOY_DIR_PATH})
#
#	set(QT_DIR_PATH $ENV{QTDIR})
#	string(REPLACE "\\" "/" QT_DIR_PATH ${QT_DIR_PATH})
#
#	install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DBINARY_PATH=${DEPLOY_DIR_PATH}/videoclient.exe -DQT_DIR_PATH=${QT_DIR_PATH} -P \"${CMAKE_SOURCE_DIR}/cmake/windeployqt.cmake\")")
#endif(WIN32)
