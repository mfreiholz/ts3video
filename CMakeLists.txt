cmake_minimum_required(VERSION 2.8)
project(source)

add_subdirectory(projects/networkcore)
add_subdirectory(projects/testexe)

# Qt based projects.
if(QTDIR)
    message("Qt found")
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
endif()
add_subdirectory(projects/qcorprotocollib)
add_subdirectory(projects/qtestexe)
