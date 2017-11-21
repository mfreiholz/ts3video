cmake_minimum_required(VERSION 2.8)
project(videoserver)

cmake_policy(SET CMP0020 NEW)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED) # Due to videolib
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
#find_package(Qt5Sql REQUIRED)

# Sources
set(headers
	src/servercliententity.h
	src/serverchannelentity.h
	src/virtualserveroptions.h
	src/action/actionbase.h
	src/action/channels.h
	src/action/media.h
	src/statistics/logserverstatisticstask.h
)

set(qheaders
	src/virtualserver.h
	src/clientconnectionhandler.h
	src/mediasockethandler.h
	src/websocketstatusserver.h
)

set(sources
	src/main.cpp
	src/virtualserveroptions.cpp
	src/virtualserver.cpp
	src/clientconnectionhandler.cpp
	src/mediasockethandler.cpp
	src/websocketstatusserver.cpp
	src/servercliententity.cpp
	src/serverchannelentity.cpp
	src/action/actionbase.cpp
	src/action/channels.cpp
	src/action/media.cpp
	src/statistics/logserverstatisticstask.cpp
)

set(qresources
#  res/res.qrc
)

if(WIN32)
	list(APPEND sources res/app.rc)
endif(WIN32)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qresources_wrap})

add_definitions(
	-DNOMINMAX
)

include_directories(
	${humblelogging_INCLUDE_DIRS}
)

# Target prerequisites.
if(WIN32)
	#set(win32_gui_type WIN32)
endif(WIN32)

# Target
add_executable(
	${PROJECT_NAME}
	${win32_gui_type}
	${headers}
	${sources}
	${qheaders}
	${qheaders_wrap}
	${qresources_wrap}
)

target_link_libraries(
	${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::Gui
	Qt5::WebSockets
	Qt5::Concurrent
#	Qt5::Sql
	${humblelogging_LIBRARIES}
	libdaemon
	libqtasync
	libbase
	libmediaprotocol
	libqtcorprotocol
	libqtteamspeak3
	videolib
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

if(WIN32)
	install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server)
	install(FILES ${PROJECT_SOURCE_DIR}/res/default.ini DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server/default.ini.orig)
	install(FILES ${PROJECT_SOURCE_DIR}/res/logging.conf DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server/logging.conf.orig)

	set(DEPLOY_DIR_PATH $ENV{OCS_DEPLOY_DIR_PATH}/server)
	string(REPLACE "\\" "/" DEPLOY_DIR_PATH ${DEPLOY_DIR_PATH})

	set(QT_DIR_PATH $ENV{QTDIR})
	string(REPLACE "\\" "/" QT_DIR_PATH ${QT_DIR_PATH})

	install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DBINARY_PATH=${DEPLOY_DIR_PATH} -DQT_DIR_PATH=${QT_DIR_PATH} -P \"${CMAKE_SOURCE_DIR}/cmake/windeployqt.cmake\")")
else(WIN32)
	install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server)
	install(FILES ${PROJECT_SOURCE_DIR}/res/default.ini DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server)
	install(FILES ${PROJECT_SOURCE_DIR}/res/logging.conf DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/server)
endif(WIN32)
