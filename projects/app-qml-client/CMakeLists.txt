cmake_minimum_required(VERSION 3.8)
project(app-qml-client)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml Multimedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick QuickControls2 Multimedia REQUIRED)

set(PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Logging.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Logging.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/App.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/App.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/UserInfoListModel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/UserInfoListModel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CameraVideoAdapter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CameraVideoAdapter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CameraVideoSurface.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/CameraVideoSurface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteVideoAdapter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteVideoAdapter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteVideoSurface.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteVideoSurface.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/App.qml
	${CMAKE_CURRENT_SOURCE_DIR}/src/AppSettings.qml
	${CMAKE_CURRENT_SOURCE_DIR}/src/Conference.qml
	${CMAKE_CURRENT_SOURCE_DIR}/src/CameraVideo.qml
	${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteVideo.qml

	${CMAKE_CURRENT_SOURCE_DIR}/src/qmldir
	${CMAKE_CURRENT_SOURCE_DIR}/src/qtquickcontrols2.conf
	${CMAKE_CURRENT_SOURCE_DIR}/src/res.qrc
)

source_group(src FILES ${PROJECT_SOURCES})

add_executable(
	${PROJECT_NAME}
	${PROJECT_SOURCES}
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Quick
		Qt${QT_VERSION_MAJOR}::QuickControls2
		Qt${QT_VERSION_MAJOR}::Qml
		Qt${QT_VERSION_MAJOR}::Multimedia
		
		libclient
		libqtcorprotocol
		libmediaprotocol
		libbase
		libapp

		PRIVATE ${humblelogging_LIBRARIES}
		PRIVATE ${vpx_LIBRARIES}
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${vpx_INCLUDE_DIRS}
	PRIVATE ${humblelogging_INCLUDE_DIRS}
)


#find_package(Qt5QuickCompiler)
#qtquick_compiler_add_resources(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/res.qrc)