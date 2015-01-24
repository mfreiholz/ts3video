TEMPLATE = app
TARGET = ts3videoclient
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

HEADERS += \
  src/clientvideowidget.h \
  src/gridviewwidgetarranger.h \
  src/videocollectionwidget.h \
  src/ts3videoclient.h \
  src/ts3videoclient_p.h

SOURCES += \
  src/main.cpp \
  src/clientvideowidget.cpp \
  src/gridviewwidgetarranger.cpp \
  src/videocollectionwidget.cpp \
  src/ts3videoclient.cpp

FORMS += \
  src/clientvideowidget.ui

RESOURCES += \
  res/res.qrc
