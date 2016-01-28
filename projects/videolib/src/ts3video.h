#ifndef TS3VIDEO_H
#define TS3VIDEO_H

#include <stdint.h>

///////////////////////////////////////////////////////////////////////
// Defines from build system.
///////////////////////////////////////////////////////////////////////

#include "project-defines.h"

///////////////////////////////////////////////////////////////////////
// Common defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_SOFTWARE_VERSION_BUILD "0"                                ///< Incrementing number with each build, which can be mapped to a revision in source repository.
#define IFVS_SOFTWARE_VERSION_QSTRING QString("%1%2").arg(IFVS_SOFTWARE_VERSION).arg(IFVS_SOFTWARE_VERSION_POSTFIX)

///////////////////////////////////////////////////////////////////////
// Server specific defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS "0.6"                    ///< Comma separated list of client versions, which the current server build supports.
#define IFVS_SERVER_ADDRESS "mfreiholz.de"                             ///< The default server address. Good for testing.
#define IFVS_SERVER_CONNECTION_PORT 13370                              ///< The default server port to accept connections from clients.
#define IFVS_SERVER_MEDIA_PORT 13370                                   ///< The default server port to handle media data (e.g. video-stream).
#define IFVS_SERVER_WSSTATUS_PORT 13375                                ///< The default server status/control port (WebSocket).

///////////////////////////////////////////////////////////////////////
// Client specific defines.
///////////////////////////////////////////////////////////////////////

#define IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS "0.6"                    ///< Comma separated list of server versions, which the current client build supports.

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
