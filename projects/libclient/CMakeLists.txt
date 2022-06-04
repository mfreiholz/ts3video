CREATE_PROJECT_STATIC_QT_LIBRARY(libclient)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
#find_package(Qt5Multimedia REQUIRED)
#find_package(Qt5MultimediaWidgets REQUIRED)
#find_package(Qt5Concurrent REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE Qt5::Core
	PRIVATE Qt5::Gui
	PRIVATE Qt5::Network
	#PRIVATE Qt5::Widgets
	#PRIVATE Qt5::Multimedia
	#PRIVATE Qt5::MultimediaWidgets
	#PRIVATE Qt5::Concurrent
	#PRIVATE libqtasync
	PRIVATE libqtcorprotocol
	PRIVATE libbase
	PRIVATE libmediaprotocol
	PRIVATE libapp

	PRIVATE ${vpx_LIBRARIES}
	PRIVATE ${humblelogging_LIBRARIES}
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${humblelogging_INCLUDE_DIRS}
	PUBLIC ${vpx_INCLUDE_DIRS}
)
