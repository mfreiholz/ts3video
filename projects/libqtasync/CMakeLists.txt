CREATE_PROJECT_STATIC_QT_LIBRARY(libqtasync)

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt5::Core
	PUBLIC Qt5::Concurrent
)
