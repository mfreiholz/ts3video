MACRO(CREATE_PROJECT_STATIC_LIBRARY name_)
	cmake_minimum_required(VERSION 3.8)
	project(${name_})

	### Sources

	file(GLOB_RECURSE headers ${PROJECT_NAME}/*.h)
	file(GLOB_RECURSE sources_cpp ${PROJECT_NAME}/*.cpp)
	file(GLOB_RECURSE sources_c ${PROJECT_NAME}/*.c)

	set(sources "")
	list(APPEND sources ${sources_cpp} ${sources_c})

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${headers} ${sources}
	)

	### Binaries

	add_library(
		${PROJECT_NAME}
		STATIC
		${headers}
		${sources}
	)

	target_include_directories(
		${PROJECT_NAME}
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	)
ENDMACRO()

#######################################################################

MACRO(CREATE_PROJECT_STATIC_QT_LIBRARY name_)
	cmake_minimum_required(VERSION 3.8)
	project(${name_})
	
	### Qt

	cmake_policy(SET CMP0020 NEW)

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTORCC ON)

	### Sources

	file(GLOB_RECURSE headers ${PROJECT_NAME}/*.h)
	file(GLOB_RECURSE sources_cpp ${PROJECT_NAME}/*.cpp)
	file(GLOB_RECURSE sources_c ${PROJECT_NAME}/*.c)
	file(GLOB_RECURSE qtforms ${PROJECT_NAME}/*.ui)
	file(GLOB_RECURSE qtres ${PROJECT_NAME}/*.qrc)

	set(sources "")
	list(APPEND sources ${sources_cpp} ${sources_c})

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${headers} ${sources}
	)

	### Binaries

	add_library(
		${PROJECT_NAME}
		STATIC
		${headers}
		${sources}
		${qtforms}
		${qtres}
	)

	target_include_directories(
		${PROJECT_NAME}
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	)
ENDMACRO()
