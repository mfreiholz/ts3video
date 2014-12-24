cmake_minimum_required(VERSION 2.8)
project(source)

add_subdirectory(projects/corlib) # TCP protocol lib
add_subdirectory(projects/medlib) # UDP protocol lib
add_subdirectory(projects/testexe)

# Qt based projects.
if(QTDIR)
    message("Qt found")
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
endif()
add_subdirectory(projects/qcorlib)
add_subdirectory(projects/qtestexe)
