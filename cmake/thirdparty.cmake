
# ---------------------------------------------------------------------
# Third party (HumbleLogging)
# ---------------------------------------------------------------------

#add_subdirectory(thirdparty/humblelogging)
#set(humblelogging_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/thirdparty/humblelogging/include)
#macro(humblelogging_POSTBUILD target_)
#  if(WIN32)
#    add_custom_command(
#      TARGET ${target_} POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE_DIR:humblelogging>/humblelogging.dll
#        $<TARGET_FILE_DIR:${target_}>/humblelogging.dll
#    )
#  else(WIN32)
#    add_custom_command(
#      TARGET ${target_} POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE_DIR:humblelogging>/libhumblelogging.so
#        $<TARGET_FILE_DIR:${target_}>/libhumblelogging.so
#    )
#  endif(WIN32)
#endmacro(humblelogging_POSTBUILD)

# ---------------------------------------------------------------------
# Third party (VPX)
# ---------------------------------------------------------------------

set(vpx_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/vpx/include
)
if(WIN32)
  set(vpx_LIBRARIES
    optimized "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-release/vpxmd.lib"
    debug "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc12-debug/vpxmdd.lib"
  )
endif(WIN32)

# ---------------------------------------------------------------------
# Third party (Opus)
# ---------------------------------------------------------------------

set(opus_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/opus/include
)
if(WIN32)
  set(opus_LIBRARIES
    optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc12-release/opus.lib"
    debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc12-debug/opus.lib"
  )
endif(WIN32)

# ---------------------------------------------------------------------
# Third party (ts3pluginsdk)
# ---------------------------------------------------------------------

set(ts3pluginsdk_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/teamspeak3-clientpluginsdk/pluginsdk-3.0.16/include
)
