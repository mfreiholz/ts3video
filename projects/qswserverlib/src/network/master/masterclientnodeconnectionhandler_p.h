#ifndef MASTERCLIENTNODECONNECTIONHANDLERPRIVATE_HEADER
#define MASTERCLIENTNODECONNECTIONHANDLERPRIVATE_HEADER

#include "QObject"
#include "QAbstractSocket"
#include "masterclientnodeconnectionhandler.h"
#include "../tcpprotocol.h"
class QTcpSocket;
class MasterServer;


class MasterClientNodeConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MasterClientNodeConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState);
  void onSocketReadyRead();

public:
  void processRequest(TCP::Request &request);

public:
  MasterClientNodeConnectionHandler *_owner;
  MasterServer *_server;
  QTcpSocket *_socket;
  TCP::ProtocolHandler _protocol;
};


#endif