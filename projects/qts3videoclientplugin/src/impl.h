#ifndef IMPL_H
#define IMPL_H

#include "public_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * Public C-API
   * @version 1.0
   */
  typedef struct {
    uint64 clientId;
    uint64 channelId;
    char serverAddress[MAX_PATH];
    uint64 serverPort;
  } TS3Data;

  /**
   * Starts the video-app in an external process and automatically joins the given channel defined in <em>ts3data</em>.
   * @param serverAddress IP address of the server.
   * @param serverPort Port on which the server listens for new video connections.
   * @param username Display name to show in video-app.
   * @param ts3data Basic information from TS3, which are used to join the correct channel.
   * @return 0 = OK; Everything else may indicate an error.
   */
  int runClient(const char *serverAddress, unsigned short serverPort, const char *username, TS3Data *ts3data);

#ifdef __cplusplus
}
#endif

#endif