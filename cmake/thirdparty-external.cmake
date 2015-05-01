include(ExternalProject)

# HumbleLogging
ExternalProject_Add(
  humblelogging

  GIT_REPOSITORY "https://github.com/mfreiholz/humble-logging-library.git"
  GIT_TAG "master"

  UPDATE_COMMAND ""
  PATCH_COMMAND ""

  SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/humblelogging-external"
  CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF

  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

#ExternalProject_Add_Step(
#  humblelogging CopyToBin
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/humblelogging/bin ${GLOBAL_OUTPUT_PATH}
#  COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/humblelogging/lib ${GLOBAL_OUTPUT_PATH}
#  DEPENDEES install
#)

set(humblelogging_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdparty/humblelogging-external/include")
set(humblelogging_LIBRARIES "$<TARGET_FILE_NAME:humblelogging>")
