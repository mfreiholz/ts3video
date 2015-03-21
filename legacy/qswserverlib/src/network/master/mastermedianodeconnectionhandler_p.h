#ifndef MASTERMEDIANODECONNECTIONHANDLERPRIVATE_HEADER
#define MASTERMEDIANODECONNECTIONHANDLERPRIVATE_HEADER

#include "QObject"
#include "QAbstractSocket"
#include "mastermedianodeconnectionhandler.h"
#include "../tcpprotocol.h"
class QTcpSocket;


class MasterMediaNodeConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MasterMediaNodeConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onSocketReadyRead();

protected:
  void processRequest(TCP::Request &request);

public:
  MasterMediaNodeConnectionHandler *_owner;
  MasterServer *_server;
  QTcpSocket *_socket;
  TCP::ProtocolHandler _protocol;
  MasterMediaNodeConnectionHandler::NodeStatus _nodeStatus;
};

#endif