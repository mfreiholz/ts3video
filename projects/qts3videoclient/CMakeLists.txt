cmake_minimum_required(VERSION 2.8)
project(qts3videoclient)
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
  src/cameraframegrabber.h
  src/videowidget.h
  src/videowidget_p.h
  src/gridviewwidgetarranger.h
  src/videocollectionwidget.h
  src/ts3videoclient.h
  src/ts3videoclient_p.h
  src/clientapplogic.h
  src/clientapplogic_p.h
  src/clientcameravideowidget.h
  src/udpvideoframedecoder.h
  src/remoteclientvideowidget.h
  src/vp8encoder.h
  src/vp8decoder.h
  src/startupwidget.h
  src/hangoutviewwidget.h
  src/hangoutviewwidget_p.h
  src/tileviewwidget.h
  src/tileviewwidget_p.h
  src/flowlayout.h
  src/aboutwidget.h
  src/movablewidgetcontainer.h
  src/hintoverlaywidget.h
  src/hintoverlaywidget_p.h
  src/model/clientlistmodel.h
  src/model/clientlistmodel_p.h
)

set(sources
  src/main.cpp
  src/cameraframegrabber.cpp
  src/videowidget.cpp
  src/gridviewwidgetarranger.cpp
  src/videocollectionwidget.cpp
  src/ts3videoclient.cpp
  src/clientapplogic.cpp
  src/clientcameravideowidget.cpp
  src/udpvideoframedecoder.cpp
  src/remoteclientvideowidget.cpp
  src/vp8encoder.cpp
  src/vp8decoder.cpp
  src/startupwidget.cpp
  src/hangoutviewwidget.cpp
  src/viewbase.cpp
  src/tileviewwidget.cpp
  src/flowlayout.cpp
  src/aboutwidget.cpp
  src/movablewidgetcontainer.cpp
  src/hintoverlaywidget.cpp
  src/model/clientlistmodel.cpp
)

set(qforms
  src/startupwidget.ui
  src/aboutwidget.ui
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
  ${humblelogging_INCLUDE_DIRS}
  ../medlib/include
  ../qcorlib/src
  ../qts3videocorelib/src
  ${vpx_INCLUDE_DIRS}
)

# Target
add_executable(
  ${PROJECT_NAME}
  WIN32
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
  qcorlib
  qts3videocorelib
  medlib
  ${vpx_LIBRARIES}
  humblelogging
)

if(IncludeOpenGLSupport)
  target_link_libraries(${PROJECT_NAME} opengl32) # TODO This shouldn't be here!
endif()

set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "videoclient"
)

humblelogging_POSTBUILD(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
