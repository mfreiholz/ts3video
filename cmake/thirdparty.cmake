
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

if(WIN32)
	set(vpx_INCLUDE_DIRS
	  #${PROJECT_SOURCE_DIR}/thirdparty/vpx/include
	  "C:/Users/Manuel/Sources/_libs/libvpx-builds/${ARCH}-vc12/include"
	)
	set(vpx_LIBRARIES
		#optimized "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc14-md-release/vpxmd.lib"
		#debug "${PROJECT_SOURCE_DIR}/thirdparty/vpx/lib-win-${ARCH}-vc14-md-debug/vpxmdd.lib"
		optimized "C:/Users/Manuel/Sources/_libs/libvpx-builds/${ARCH}-vc12/lib/vpxmd.lib"
		debug     "C:/Users/Manuel/Sources/_libs/libvpx-builds/${ARCH}-vc12/lib/vpxmdd.lib"
	)
else(WIN32)
	set(vpx_INCLUDE_DIRS
	  "/home/manuel/Libraries/libvpx-x86-64-gcc521/install/include"
	)
	set(vpx_LIBRARIES
		"/home/manuel/Libraries/libvpx-x86-64-gcc521/install/lib/libvpx.a"
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
    optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-release/celt.lib"
	optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-release/opus.lib"
	optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-release/silk_common.lib"
	optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-release/silk_fixed.lib"
	optimized "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-release/silk_float.lib"
    debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-debug/celt.lib"
	debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-debug/opus.lib"
	debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-debug/silk_common.lib"
	debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-debug/silk_fixed.lib"
	debug "${PROJECT_SOURCE_DIR}/thirdparty/opus/lib-win-${ARCH}-vc14-md-debug/silk_float.lib"
  )
endif(WIN32)

# ---------------------------------------------------------------------
# Third party (ts3pluginsdk)
# ---------------------------------------------------------------------

set(ts3pluginsdk_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/thirdparty/teamspeak3-clientpluginsdk/pluginsdk-3.0.16/include
)
