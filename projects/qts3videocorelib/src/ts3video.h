#ifndef TS3VIDEO_H
#define TS3VIDEO_H

/*!
  Global definitions for the project.

  Shortcuts:
    "IFVS_*" stands for "insaneFactory Video Server ..."
 */
#define IFVS_SOFTWARE_VERSION "2.0"               ///< Version of the software (Always use <MAJOR>.<MINOR> format!)
#define IFVS_SOFTWARE_VERSION_POSTFIX "beta"      ///< Version label (alpha, beta, release, ...)
#define IFVS_SOFTWARE_VERSION_BUILD "0"           ///< Incrementing number with each build, which can be mapped to a revision in source repository.
#define IFVS_SOFTWARE_VERSION_QSTRING QString("%1 %2").arg(IFVS_SOFTWARE_VERSION).arg(IFVS_SOFTWARE_VERSION_POSTFIX).arg(IFVS_SOFTWARE_VERSION_BUILD)

// Server specific defines.
#define IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS "2.0"      ///< Comma separated list of client versions, which the current server build supports.
#define IFVS_SERVER_ADDRESS "h2377348.stratoserver.net"  ///< The default server address. Good for testing.
#define IFVS_SERVER_CONNECTION_PORT 13370                ///< The default server port to accept connections from clients.
#define IFVS_SERVER_MEDIA_PORT 13370                     ///< The default server port to handle media data (e.g. video-stream).
#define IFVS_SERVER_WSSTATUS_PORT 13375                  ///< The default server status/control port (WebSocket).

// Client specific defines.
#define IFVS_CLIENT_VIDEO_SIZE 640, 480         ///< Keep 4:3 ratio (e.g.: 960x720, 640x480, 480x360)

#endif