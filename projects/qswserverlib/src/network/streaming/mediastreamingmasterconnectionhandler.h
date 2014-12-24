#ifndef MEDIASTREAMINGMASTERCONNECTIONHANDLER_HEADER
#define MEDIASTREAMINGMASTERCONNECTIONHANDLER_HEADER

#include "QObject"
#include "QHostAddress"
#include "mediaserializables.h"
class MediaStreamingServer;


class MediaStreamingMasterConnectionHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  struct StartOptions {
    QHostAddress address;
    quint16 port;

    QHostAddress externalStreamingAddress;
    quint16 externalStreamingPort;
  };

  MediaStreamingMasterConnectionHandler(MediaStreamingServer *server, QObject *parent = 0);
  virtual ~MediaStreamingMasterConnectionHandler();
  void startup(const StartOptions &opts);
  void shutdown();

signals:
  void disconnected();
  void clientsUpdated(const UdpDataReceiverMap &map);
};


#endif