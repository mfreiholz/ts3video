CREATE_PROJECT_STATIC_QT_LIBRARY(libqtcorprotocol)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC libcorprotocol
	PUBLIC Qt5::Core
	PUBLIC Qt5::Network
)
