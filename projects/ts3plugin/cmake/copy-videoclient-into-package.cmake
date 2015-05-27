# Copies videoclient into "ts3_plugin/plugins/ts3video/<version>/.here."
#message("TEST BEGIN")
#message($<TARGET_FILE:qts3videoclient>)
#message(${TS3_PLUGIN_PACKAGE_DIR})
#message("TEST END")

if(WIN32)
  file(
    COPY
      ${CMAKE_INSTALL_PREFIX}/bin/videoclient.exe
      ${CMAKE_INSTALL_PREFIX}/bin/humblelogging.dll
    DESTINATION
      ${CMAKE_INSTALL_PREFIX}/ts3_plugin/plugins/ts3video/0.1/
  )
endif(WIN32)