# ---------------------------------------------------------------------
# Third party (VPX)
# ---------------------------------------------------------------------

if(WIN32)
	set(vpx_base_dir ${PROJECT_SOURCE_DIR}/thirdparty/vendor-libvpx/win-${ARCH}-vc15)
	set(vpx_INCLUDE_DIRS ${vpx_base_dir}/include)
	set(vpx_LIBRARIES optimized ${vpx_base_dir}/release/vpxmd.lib debug ${vpx_base_dir}/debug/vpxmdd.lib)
else(WIN32)
	set(vpx_base_dir ${PROJECT_SOURCE_DIR}/thirdparty/vendor-libvpx/linux-${ARCH}-gcc)
	set(vpx_INCLUDE_DIRS ${vpx_base_dir}/include)
	set(vpx_LIBRARIES optimized ${vpx_base_dir}/lib/libvpx.a debug ${vpx_base_dir}/lib/libvpx.a)
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
	${PROJECT_SOURCE_DIR}/thirdparty/teamspeak3-clientpluginsdk/pluginsdk-3.1.1.1/include
)