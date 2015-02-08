
option(IncludeOpenGLSupport "IncludeOpenGLSupport" ON)

# Qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

# Sources
set(headers
)

set(qheaders
  src/videowidget.h
  src/videowidget_p.h
  src/gridviewwidgetarranger.h
  src/videocollectionwidget.h
  src/ts3videoclient.h
  src/ts3videoclient_p.h
  src/clientapplogic.h
  src/clientcameravideowidget.h
  src/udpvideoframedecoder.h
  src/remoteclientvideowidget.h
  src/vp8encoder.h
  src/vp8decoder.h
)

set(sources
  src/main.cpp
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
)

set(qforms
)

set(qresources
  res/res.qrc
)

if(IncludeOpenGLSupport)
  add_definitions(-DUSE_OPENGL_IMPL)
  #add_definitions(-DINCLUDE_OPENGL_VIDEOWIDGET_SUPPORT)
  add_definitions(-DINCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT)
  list(APPEND qheaders src/opengl/openglrenderthread.h)
  list(APPEND qheaders src/opengl/openglwindow.h)
  list(APPEND sources src/opengl/openglrenderthread.cpp)
  list(APPEND sources src/opengl/openglwindow.cpp)

  list(APPEND qheaders src/opengl2/yuvvideowindow.h)
  list(APPEND sources src/opengl2/yuvvideowindow.cpp)
endif()

qt5_wrap_cpp(qheaders_wrap ${qheaders})
qt5_wrap_ui(qforms_wrap ${qforms})
qt5_add_resources(qresources_wrap ${qresources})
source_group(QtWrap FILES ${qheaders_wrap} ${qforms_wrap} ${qresources_wrap})

# Defines
add_definitions(
  -DUSE_OPENGL_IMPL
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
  qts3videoclient
  ${headers}
  ${sources}
  ${qheaders}
  ${qheaders_wrap}
  ${qforms_wrap}
  ${qresources_wrap}
)

target_link_libraries(
  qts3videoclient
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Network
  Qt5::Multimedia
  Qt5::MultimediaWidgets
  Qt5::OpenGL
  qcorlib
  qts3videocorelib
  medlib
  ${vpx_LIBRARIES}
  humblelogging
)

if(IncludeOpenGLSupport)
  target_link_libraries(qts3videoclient opengl32) # TODO This shouldn't be here!
endif()

humblelogging_POSTBUILD(qts3videoclient)