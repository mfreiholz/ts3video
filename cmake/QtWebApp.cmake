set(QTWEBAPP_PREFIX QtWebApp)

ExternalProject_Add(
	${QTWEBAPP_PREFIX}
	PREFIX ${QTWEBAPP_PREFIX}

	# Download
	URL ${CMAKE_SOURCE_DIR}/thirdparty/QtWebApp.zip

	# Update
	UPDATE_COMMAND ""
	UPDATE_DISCONNECTED 1

	# Configure
	CONFIGURE_COMMAND qmake <SOURCE_DIR>/QtWebApp/QtWebApp.pro -r -spec win32-msvc2013 CONFIG+=debug

	# Build step
	BUILD_COMMAND "nmake"

	# Install
	INSTALL_COMMAND ""
)

ExternalProject_Get_Property(${QTWEBAPP_PREFIX} SOURCE_DIR)
ExternalProject_Get_Property(${QTWEBAPP_PREFIX} BINARY_DIR)
MESSAGE(STATUS "${QTWEBAPP_PREFIX} source directory: ${SOURCE_DIR}")
MESSAGE(STATUS "${QTWEBAPP_PREFIX} binary directory: ${BINARY_DIR}")

SET(QTWEBAPP_DEFINITIONS
	-DQTWEBAPPLIB_IMPORT
)

SET(QTWEBAPP_INCLUDE_DIRS
	${SOURCE_DIR}/QtWebApp
	${BUILD_DIR}/debug
)

SET(QTWEBAPP_LINK_DIR
	${BINARY_DIR}/debug
)

SET(QTWEBAPP_LIBS
	debug ${QTWEBAPP_LINK_DIR}/QtWebAppd1.lib
)

SET(QTWEBAPP_DLLS
	${QTWEBAPP_LINK_DIR}/QtWebAppd1.dll
)

MACRO(QTWEBAPP_POSTBUILD target_)
	FOREACH(dll_abs_path ${QTWEBAPP_DLLS})
		GET_FILENAME_COMPONENT(dll_name ${dll_abs_path} NAME)
		ADD_CUSTOM_COMMAND(
			TARGET ${target_} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${dll_abs_path}"
				"$<TARGET_FILE_DIR:${target_}>/${dll_name}"
		)
	ENDFOREACH()
ENDMACRO()
