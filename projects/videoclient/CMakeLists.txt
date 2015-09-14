cmake_minimum_required(VERSION 2.8)
project(videoclient)
cmake_policy(SET CMP0020 NEW)

option(IncludeOpenGLSupport "IncludeOpenGLSupport" ON)

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
  src/viewbase.h
)

set(qheaders
  src/adminauthwidget.h
  src/cameraframegrabber.h
  src/videowidget.h
  src/videowidget_p.h
#  src/gridviewwidgetarranger.h
  src/videocollectionwidget.h
  src/clientapplogic.h
  src/clientapplogic_p.h
  src/clientcameravideowidget.h
  src/remoteclientvideowidget.h
  src/startuplogic.h
  src/startupwidget.h
#  src/hangoutviewwidget.h
#  src/hangoutviewwidget_p.h
  src/tileviewwidget.h
  src/tileviewwidget_p.h
  src/flowlayout.h
  src/aboutwidget.h
  src/movablewidgetcontainer.h
  src/hintoverlaywidget.h
  src/hintoverlaywidget_p.h
  src/camera/cameracontroller.h
  src/camera/cameracontroller_p.h
  src/networkclient/networkclient.h
  src/networkclient/networkclient_p.h
  src/networkclient/mediasocket.h
  src/networkclient/mediasocket_p.h
  src/networkclient/mediasocketthread.h
  src/networkclient/mediasocketthread_p.h
  src/networkclient/videoencodingthread.h
  src/networkclient/videodecodingthread.h
  src/networkclient/udpvideoframedecoder.h
  src/networkclient/vp8encoder.h
  src/networkclient/vp8decoder.h
  src/networkclient/clientlistmodel.h
  src/networkclient/clientlistmodel_p.h
  src/startup/ts3videostartuplogic.h
  src/ts3video/ts3videoentities.h
  src/ts3video/ts3videoupdatedialog.h
)

set(sources
  src/main.cpp
  src/adminauthwidget.cpp
  src/cameraframegrabber.cpp
  src/videowidget.cpp
#  src/gridviewwidgetarranger.cpp
  src/videocollectionwidget.cpp
  src/clientapplogic.cpp
  src/clientcameravideowidget.cpp
  src/remoteclientvideowidget.cpp
  src/startuplogic.cpp
  src/startupwidget.cpp
#  src/hangoutviewwidget.cpp
  src/viewbase.cpp
  src/tileviewwidget.cpp
  src/flowlayout.cpp
  src/aboutwidget.cpp
  src/movablewidgetcontainer.cpp
  src/hintoverlaywidget.cpp
  src/camera/cameracontroller.cpp
  src/networkclient/networkclient.cpp
  src/networkclient/mediasocket.cpp
  src/networkclient/mediasocketthread.cpp
  src/networkclient/videoencodingthread.cpp
  src/networkclient/videodecodingthread.cpp
  src/networkclient/udpvideoframedecoder.cpp
  src/networkclient/vp8encoder.cpp
  src/networkclient/vp8decoder.cpp
  src/networkclient/clientlistmodel.cpp
  src/startup/ts3videostartuplogic.cpp
  src/ts3video/ts3videoentities.cpp
  src/ts3video/ts3videoupdatedialog.cpp
)

set(qforms
  src/adminauthwidget.ui
  src/startupwidget.ui
  src/aboutwidget.ui
  src/startup/ts3videostartuplogic.ui
  src/ts3video/ts3videoupdatedialog.ui
)

set(qresources
  res/res.qrc
)

if(IncludeOpenGLSupport)
  list(APPEND qheaders src/opengl/openglrenderthread.h)
  list(APPEND qheaders src/opengl/openglwindow.h)
  list(APPEND sources src/opengl/openglrenderthread.cpp)
  list(APPEND sources src/opengl/openglwindow.cpp)

  list(APPEND qheaders src/opengl2/yuvvideowindow.h)
  list(APPEND sources src/opengl2/yuvvideowindow.cpp)

  list(APPEND qheaders src/opengl3/glvideowidget.h)
  list(APPEND qheaders src/opengl3/glvideowidget_p.h)
  list(APPEND sources src/opengl3/glvideowidget.cpp)
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
  ${humblelogging_INCLUDE_DIRS}
  ../qtasync/src
  ../medlib/include
  ../qcorlib/src
  ../videolib/src
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
  medlib
  videolib
)

if(IncludeOpenGLSupport)
  target_link_libraries(${PROJECT_NAME} opengl32) # TODO This shouldn't be here!
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
  set(INST_BINARY_DIR "client/bin/win-${ARCH}")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCC)
  set(INST_BINARY_DIR "client/bin/linux-${ARCH}")
endif(CMAKE_COMPILER_IS_GNUCC)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INST_BINARY_DIR})
