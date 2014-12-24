#ifndef MEDIASTREAMINGSERVER_HEADER
#define MEDIASTREAMINGSERVER_HEADER

#include "QObject"
#include "QString"
#include "servercore/api.h"

/*!
  Starts a single media streaming server NODE.
  It may also connect to a MASTER server. The master will update
  the NODE with sender/receiver information.
*/
class SERVERCORE_API MediaStreamingServer :
  public QObject
{
  Q_OBJECT
  friend class MediaStreamingSocketHandler;
  friend class MediaStreamingMasterConnectionHandler;
  class Private;
  Private *d;

public:
  struct Options {
    quint16 streamingPort;

    QString externalStreamingAddress; // This IP is told to the user-clients.
    quint16 externalStreamingPort; // This port is told to the user-clients.

    QString masterServerAddress;
    quint16 masterServerPort;

    Options() :
      streamingPort(6667),
      externalStreamingPort(6667),
      masterServerPort(6660)
    {}
  };

  MediaStreamingServer(const Options &opts, QObject *parent = 0);
  virtual ~MediaStreamingServer();

  bool startup();
  bool shutdown();
};


#endif