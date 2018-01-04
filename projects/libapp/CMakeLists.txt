CREATE_PROJECT_STATIC_QT_LIBRARY(libapp)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Network
	PUBLIC libbase
)
