#ifndef IMPL_H
#define IMPL_H

#include "public_definitions.h"
#include "ts3_functions.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    struct TS3Functions *funcs;

    uint64 clientId;                   ///< The ID of the client.
    char clientName[MAX_PATH];         ///< The name of the client.
    uint64 channelId;                  ///< The current channel-ID of the client.
    char serverAddress[MAX_PATH];      ///< The address of the current virtual server.
    uint64 serverPort;                 ///< The port of the current virtual server.

    uint64 targetChannelId;            ///< The ID of the channel the client wants to join.
  } TS3Data;

  /*!
    Starts the video-app in an external process and automatically joins the given channel defined in <em>ts3data</em>.

    \param ts3data Basic information from TS3, which are used to join the correct channel.
    \param skipStartupDialog 1=Skips the startup dialog and uses default values.
    \return 0 = OK; Everything else may indicate an error.
  */
  int runClient(TS3Data *ts3data, int skipStartupDialog);

#ifdef __cplusplus
}
#endif

#endif