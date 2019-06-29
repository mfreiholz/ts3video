cmake_minimum_required(VERSION 3.0)
project(ts3plugin)

cmake_policy(SET CMP0020 NEW)

set(headers
	plugin_23.h
	impl.h
)

set(sources
	plugin_23.c
	impl.cpp
)

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS
)

include_directories(
	${ts3pluginsdk_INCLUDE_DIRS}
)

add_library(
	${PROJECT_NAME}
	SHARED
	${headers}
	${sources}
)

if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ts3video_win${ARCH_BIT}")
else(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ts3video")
endif(WIN32)

#######################################################################
# Install
#######################################################################

set(DEPLOY_DIR_PATH $ENV{OCS_INSTALL_DIR_PATH}/${PROJECT_NAME})
string(REPLACE "\\" "/" DEPLOY_DIR_PATH ${DEPLOY_DIR_PATH})

if(WIN32)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${DEPLOY_DIR_PATH}/plugins)
else()
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DEPLOY_DIR_PATH}/plugins)
endif()
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -DVERSION_MAJOR=${VERSION_MAJOR} -DVERSION_MINOR=${VERSION_MINOR} -DPACKAGE_INI_DIR_PATH=${DEPLOY_DIR_PATH} -P \"${CMAKE_CURRENT_LIST_DIR}/cmake/generate-package-ini.cmake\")")
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E chdir \"${DEPLOY_DIR_PATH}\" \"${CMAKE_COMMAND}\" -E tar \"cfv\" \"ts3video.ts3_plugin\" --format=zip \"package.ini\" \"plugins\")")