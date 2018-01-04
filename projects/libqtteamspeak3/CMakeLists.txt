CREATE_PROJECT_STATIC_QT_LIBRARY(libqtteamspeak3)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${humblelogging_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Network
	PRIVATE ${humblelogging_LIBRARIES}
)
