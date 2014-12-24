#ifndef MASTERMEDIANODECONNECTIONHANDLER_HEADER
#define MASTERMEDIANODECONNECTIONHANDLER_HEADER

#include "QObject"
#include "QString"
#include "QDateTime"
#include "network/streaming/mediaserializables.h"
class QTcpSocket;
class MasterServer;


class MasterMediaNodeConnectionHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  struct NodeStatus {
    QDateTime lastUpdate;
    QString externalStreamingAddress;
    quint16 externalStreamingPort;
    quint64 currentBandwidth;
    quint64 availableBandwidth;
  };

  MasterMediaNodeConnectionHandler(MasterServer *server, QTcpSocket *socket, QObject *parent = 0);
  virtual ~MasterMediaNodeConnectionHandler();
  QTcpSocket* getSocket() const;
  const NodeStatus& getNodeStatus() const;

public slots:
  void sendUpdateClients(const UdpDataReceiverMap &map);

signals:
  void disconnected();
};


#endif