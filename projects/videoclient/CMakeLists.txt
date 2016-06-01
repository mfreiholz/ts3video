cmake_minimum_required(VERSION 2.8)
project(videoclient)
cmake_policy(SET CMP0020 NEW)

option(IncludeOpenGLSupport "IncludeOpenGLSupport" ON)
option(IncludeAudioSupport "IncludeAudioSupport" OFF)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Concurrent REQUIRED)

# Sources
set(headers
	src/startup/startuplogic.h
	src/util/qwidgetutil.h
)

set(qheaders
	src/admin/adminwidget.h
	src/video/conferencevideowindow.h
	src/video/conferencevideowindowsidebar.h
	src/video/userlistwidget.h
	src/video/cameraframegrabber.h
	src/video/clientcameravideowidget.h
	src/video/remoteclientvideowidget.h
	src/video/tileviewwidget.h
	src/video/tileviewwidget_p.h
	src/video/videowidget.h
	src/video/videowidget_p.h
	src/video/videosettingswidget.h
	src/startup/startupwidget.h
	src/startup/devstartuplogic.h
	src/adminauthwidget.h
	src/flowlayout.h
	src/aboutwidget.h
	src/movablewidgetcontainer.h
	src/hintoverlaywidget.h
	src/proxysettingswidget.h
	src/networkclient/networkclient.h
	src/networkclient/networkclient_p.h
	src/networkclient/mediasocket.h
	src/networkclient/mediasocket_p.h
	src/networkclient/videoencodingthread.h
	src/networkclient/videodecodingthread.h
	src/networkclient/udpvideoframedecoder.h
	src/networkclient/vp8encoder.h
	src/networkclient/vp8decoder.h
	src/networkclient/clientlistmodel.h
	src/networkclient/clientlistmodel_p.h
	src/ts3video/ts3videostartuplogic.h
	src/ts3video/ts3videoentities.h
	src/ts3video/ts3videoupdatedialog.h
	src/test/connectionteststartuplogic.h
	src/test/directconnectstartuplogic.h
)

set(sources
	src/main.cpp
	src/admin/adminwidget.cpp
	src/video/conferencevideowindow.cpp
	src/video/conferencevideowindowsidebar.cpp
	src/video/userlistwidget.cpp
	src/video/cameraframegrabber.cpp
	src/video/clientcameravideowidget.cpp
	src/video/remoteclientvideowidget.cpp
	src/video/tileviewwidget.cpp
	src/video/videowidget.cpp
	src/video/videosettingswidget.cpp
	src/startup/startuplogic.cpp
	src/startup/startupwidget.cpp
	src/startup/devstartuplogic.cpp
	src/adminauthwidget.cpp
	src/flowlayout.cpp
	src/aboutwidget.cpp
	src/movablewidgetcontainer.cpp
	src/hintoverlaywidget.cpp
	src/proxysettingswidget.cpp
	src/networkclient/networkclient.cpp
	src/networkclient/mediasocket.cpp
	src/networkclient/videoencodingthread.cpp
	src/networkclient/videodecodingthread.cpp
	src/networkclient/udpvideoframedecoder.cpp
	src/networkclient/vp8encoder.cpp
	src/networkclient/vp8decoder.cpp
	src/networkclient/clientlistmodel.cpp
	src/ts3video/ts3videostartuplogic.cpp
	src/ts3video/ts3videoentities.cpp
	src/ts3video/ts3videoupdatedialog.cpp
	src/util/qwidgetutil.cpp
	src/test/connectionteststartuplogic.cpp
	src/test/directconnectstartuplogic.cpp
)

set(qforms
	src/admin/adminwidget.ui
	src/adminauthwidget.ui
	src/startup/startupwidget.ui
	src/aboutwidget.ui
	src/proxysettingswidget.ui
	src/ts3video/ts3videostartuplogic.ui
	src/ts3video/ts3videoupdatedialog.ui
	src/video/videosettingswidget.ui
)

set(qresources
	res/res.qrc
)

if(IncludeOpenGLSupport)
	list(APPEND qheaders src/video/opengl/openglrenderthread.h)
	list(APPEND qheaders src/video/opengl/openglwindow.h)
	list(APPEND sources src/video/opengl/openglrenderthread.cpp)
	list(APPEND sources src/video/opengl/openglwindow.cpp)

	list(APPEND qheaders src/video/opengl2/yuvvideowindow.h)
	list(APPEND sources src/video/opengl2/yuvvideowindow.cpp)

	list(APPEND qheaders src/video/opengl3/glvideowidget.h)
	list(APPEND headers src/video/opengl3/glvideowidget_p.h)
	list(APPEND sources src/video/opengl3/glvideowidget.cpp)
endif()

if(IncludeAudioSupport)
	add_definitions(-DOCS_INCLUDE_AUDIO)

	list(APPEND qheaders src/audio/audioframegrabber.h)
	list(APPEND qheaders src/audio/audioframeplayer.h)
	list(APPEND qheaders src/networkclient/audioencodingthread.h)
	list(APPEND qheaders src/networkclient/audiodecodingthread.h)
	list(APPEND qheaders src/networkclient/audioudpdecoder.h)
	list(APPEND qheaders src/networkclient/opusencoder.h)
	list(APPEND qheaders src/networkclient/opusdecoder.h)

	list(APPEND sources src/audio/audioframegrabber.cpp)
	list(APPEND sources src/audio/audioframeplayer.cpp)
	list(APPEND sources src/networkclient/audioencodingthread.cpp)
	list(APPEND sources src/networkclient/audiodecodingthread.cpp)
	list(APPEND sources src/networkclient/audioudpdecoder.cpp)
	list(APPEND sources src/networkclient/opusencoder.cpp)
	list(APPEND sources src/networkclient/opusdecoder.cpp)
endif()

if(WIN32)
	list(APPEND sources res/app.rc)
endif(WIN32)

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
)

# Includes
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${vpx_INCLUDE_DIRS}
	${opus_INCLUDE_DIRS}
	${humblelogging_INCLUDE_DIRS}
	../qtasync/include
	../baselib/include
	../medlib/include
	../qcorlib/include
	../videolib/src
	./src
)

# Target prerequisites.
if(WIN32)
	set(win32_gui_type WIN32)
endif(WIN32)

# Target
add_executable(
	${PROJECT_NAME}
	${win32_gui_type}
	${headers}
	${sources}
	${qheaders}
	${qheaders_wrap}
	${qforms_wrap}
	${qresources_wrap}
)

target_link_libraries(
	${PROJECT_NAME}
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::Network
	Qt5::Multimedia
	Qt5::MultimediaWidgets
	Qt5::OpenGL
	Qt5::Concurrent
	${vpx_LIBRARIES}
	${humblelogging_LIBRARIES}
	qtasync
	qcorlib
	baselib
	medlib
	videolib
)

if(IncludeOpenGLSupport)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} opengl32) # TODO This shouldn't be here!
	else()
		target_link_libraries(${PROJECT_NAME} /usr/lib/nvidia-352/libGL.so)
		target_link_libraries(${PROJECT_NAME} pthread)
	endif()
endif()

if(IncludeAudioSupport)
	target_link_libraries(${PROJECT_NAME} ${opus_LIBRARIES})
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

if(WIN32)
	install(TARGETS ${PROJECT_NAME} DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/client)
	install(FILES ${PROJECT_SOURCE_DIR}/res/default.ini DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/client)
	install(FILES ${PROJECT_SOURCE_DIR}/res/logging.conf DESTINATION $ENV{OCS_DEPLOY_DIR_PATH}/client)

	set(DEPLOY_DIR_PATH $ENV{OCS_DEPLOY_DIR_PATH}/client)
	string(REPLACE "\\" "/" DEPLOY_DIR_PATH ${DEPLOY_DIR_PATH})

	set(QT_DIR_PATH $ENV{QTDIR})
	string(REPLACE "\\" "/" QT_DIR_PATH ${QT_DIR_PATH})

	install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DBINARY_PATH=${DEPLOY_DIR_PATH} -DQT_DIR_PATH=${QT_DIR_PATH} -P \"${CMAKE_SOURCE_DIR}/cmake/windeployqt.cmake\")")
else(WIN32)
	# ToDo Linux steps...
endif(WIN32)
