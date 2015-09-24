#ifndef TS3VIDEO_H
#define TS3VIDEO_H

///////////////////////////////////////////////////////////////////////
// Defines from build system.
///////////////////////////////////////////////////////////////////////

#include "project-defines.h"
//#define IFVS_SOFTWARE_VERSION "0.2"                                  ///< Version of the software (Always use <MAJOR>.<MINOR> format!)
//#define IFVS_SOFTWARE_VERSION_POSTFIX "beta"                         ///< Version label (alpha, beta, release, ...)

///////////////////////////////////////////////////////////////////////
// Common defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_SOFTWARE_VERSION_BUILD "0"                                ///< Incrementing number with each build, which can be mapped to a revision in source repository.
#define IFVS_SOFTWARE_VERSION_QSTRING QString("%1%2").arg(IFVS_SOFTWARE_VERSION).arg(IFVS_SOFTWARE_VERSION_POSTFIX)

///////////////////////////////////////////////////////////////////////
// Server specific defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS "0.4,0.5"                ///< Comma separated list of client versions, which the current server build supports.
#define IFVS_SERVER_ADDRESS "teamspeak.insanefactory.com"              ///< The default server address. Good for testing.
#define IFVS_SERVER_CONNECTION_PORT 13370                              ///< The default server port to accept connections from clients.
#define IFVS_SERVER_MEDIA_PORT 13370                                   ///< The default server port to handle media data (e.g. video-stream).
#define IFVS_SERVER_WSSTATUS_PORT 13375                                ///< The default server status/control port (WebSocket).

///////////////////////////////////////////////////////////////////////
// Client specific defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS "0.4,0.5"                ///< Comma separated list of server versions, which the current client build supports.
#define IFVS_CLIENT_VIDEO_SIZE 640, 480                                ///< Keep 4:3 ratio (e.g.: 960x720, 640x480, 480x360)

///////////////////////////////////////////////////////////////////////
// Important global datatypes.
///////////////////////////////////////////////////////////////////////

typedef int vs_clientid_t;
typedef int vs_channelid_t;

///////////////////////////////////////////////////////////////////////
// Status Codes
///////////////////////////////////////////////////////////////////////

#define CREATE_SERVER_STATUS(NAME, VALUE, DESCR) const static int IFVS_STATUS_##NAME = VALUE

CREATE_SERVER_STATUS(OK, 0, "");
CREATE_SERVER_STATUS(INTERNAL_ERROR, 500, "Internal error.");
CREATE_SERVER_STATUS(NOT_IMPLEMENTED, 501, "Not implemented.");
CREATE_SERVER_STATUS(INCOMPATIBLE_VERSION, 505, "");
CREATE_SERVER_STATUS(CONNECTION_LIMIT_REACHED, 429, "");
CREATE_SERVER_STATUS(CONNECTION_TIMEOUT, 504, "");
CREATE_SERVER_STATUS(BANDWIDTH_LIMIT_REACHED, 509, "");
CREATE_SERVER_STATUS(UNAUTHORIZED, 401, "");
CREATE_SERVER_STATUS(FORBIDDEN, 403, "");
CREATE_SERVER_STATUS(AUTHENTICATION_TIMEOUT, 419, "");
CREATE_SERVER_STATUS(INVALID_PARAMETERS, 701, "");
CREATE_SERVER_STATUS(BANNED, 702, "");

#endif
