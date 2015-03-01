
# Third party (HumbleLogging)
add_subdirectory(thirdparty/humblelogging)
set(humblelogging_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/humblelogging/include)
#set(humblelogging_LIBRARIES humblelogging)
macro(humblelogging_POSTBUILD target_)
  add_custom_command(
    TARGET ${target_} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      #"${PROJECT_SOURCE_DIR}/build/thirdparty/humblelogging/bin/$<CONFIGURATION>/humblelogging.dll"
      $<TARGET_FILE_DIR:humblelogging>/humblelogging.dll
      $<TARGET_FILE_DIR:${target_}>/humblelogging.dll
  )
endmacro(humblelogging_POSTBUILD)

# Third party (VPX)
set(vpx_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/vpx/include
)
set(vpx_LIBRARIES
  optimized "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-release/vpxmd.lib"
  debug "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-debug/vpxmdd.lib"
)

# Third party (ts3pluginsdk)
set(ts3pluginsdk_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/teamspeak3-clientpluginsdk/pluginsdk-3.0.16/include
)
