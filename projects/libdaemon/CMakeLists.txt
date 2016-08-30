cmake_minimum_required(VERSION 2.8)
project(libdaemon)

set(headers
	api.h
)

set(sources
	main.c
)

if(WIN32)
	list(APPEND sources impl_windows.c)
else(WIN32)
	list(APPEND sources impl_linux.c)
endif(WIN32)

add_library(
	${PROJECT_NAME}
	STATIC
	${headers}
	${sources}
)