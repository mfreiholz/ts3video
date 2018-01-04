CREATE_PROJECT_STATIC_LIBRARY(libcorprotocol)

if(WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE Ws2_32.dll
	)
endif(WIN32)
