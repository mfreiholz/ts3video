cmake_minimum_required(VERSION 3.8)
project(videoserver)

### Qt

cmake_policy(SET CMP0020 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED) # Due to videolib
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Concurrent REQUIRED)

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
	#set(GUI_TYPE WIN32)
endif()

add_executable(
	${PROJECT_NAME}
	${GUI_TYPE}
	${headers}
	${sources}
	${qtforms}
	${qtres}
)

# add_definitions(
	# -DNOMINMAX
# )

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${humblelogging_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE ${humblelogging_LIBRARIES}
	PRIVATE Qt5::Core
	PRIVATE Qt5::Network
	PRIVATE Qt5::Gui
	PRIVATE Qt5::WebSockets
	PRIVATE Qt5::Concurrent
	PRIVATE libdaemon
	PRIVATE libqtasync
	PRIVATE libbase
	PRIVATE libmediaprotocol
	PRIVATE libqtcorprotocol
	PRIVATE libqtteamspeak3
	PRIVATE libapp
)

set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME "videoserver"
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

install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/server)
install(FILES ${PROJECT_SOURCE_DIR}/res/default.ini DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/server RENAME default.ini.orig)
install(FILES ${PROJECT_SOURCE_DIR}/res/logging.conf DESTINATION $ENV{OCS_INSTALL_DIR_PATH}/server RENAME logging.conf.orig)

#if(WIN32)
#	set(DEPLOY_DIR_PATH $ENV{OCS_INSTALL_DIR_PATH}/server)
#	string(REPLACE "\\" "/" DEPLOY_DIR_PATH ${DEPLOY_DIR_PATH})

#	set(QT_DIR_PATH $ENV{QTDIR})
#	string(REPLACE "\\" "/" QT_DIR_PATH ${QT_DIR_PATH})

#	install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DBINARY_PATH=${DEPLOY_DIR_PATH}/videoserver.exe -DQT_DIR_PATH=${QT_DIR_PATH} -P \"${CMAKE_SOURCE_DIR}/cmake/windeployqt.cmake\")")
#endif(WIN32)