#ifndef MASTERCLIENTNODECONNECTIONHANDLER_HEADER
#define MASTERCLIENTNODECONNECTIONHANDLER_HEADER

#include "QObject"
class QTcpSocket;
class MasterServer;


class MasterClientNodeConnectionHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  MasterClientNodeConnectionHandler(MasterServer *server, QTcpSocket *socket, QObject *parent = 0);
  virtual ~MasterClientNodeConnectionHandler();

signals:
  void disconnected();
};


#endif