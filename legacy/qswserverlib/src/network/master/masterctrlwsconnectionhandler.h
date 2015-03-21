#ifndef MASTERCTRLWSCONNECTIONHANDLER_HEADER
#define MASTERCTRLWSCONNECTIONHANDLER_HEADER

#include "QObject"
class QWebSocket;
class MasterServer;

class MasterCtrlWsConnectionHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  MasterCtrlWsConnectionHandler(MasterServer *master, QWebSocket *socket, QObject *parent = 0);
  virtual ~MasterCtrlWsConnectionHandler();

signals:
  void disconnected();
};

#endif