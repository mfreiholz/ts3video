#ifndef MASTERSERVERPRIVATE_HEADER
#define MASTERSERVERPRIVATE_HEADER

#include "QObject"
#include "QList"
#include "QTcpServer"
#include "QWebSocketServer"
#include "masterserver.h"
class MasterMediaNodeConnectionHandler;
class MasterClientNodeConnectionHandler;
class MasterCtrlWsConnectionHandler;


class MasterServer::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MasterServer *owner);
  virtual ~Private();
  bool startup();
  bool shutdown();

public slots:
  void onNewMediaStreamingNodeConnection();
  void onMediaStreamingNodeDisconnected();

  void onNewClientNodeConnection();
  void onClientNodeDisconnected();

  void onNewCtrlWsConnection();
  void onCtrlWsDisconnected();

public:
  MasterServer *_owner;
  MasterServer::Options _opts;

  // Attributes to manage Master<->Media-Node connections.
  QTcpServer _mediaNodesServer;
  QList<MasterMediaNodeConnectionHandler*> _mediaNodeConnections;

  // Attributes to manage Master<->Client-Node connections.
  QTcpServer _clientNodesServer;
  QList<MasterClientNodeConnectionHandler*> _clientNodeConnections;

  // Attributes to manage Master's WebSocket.
  QWebSocketServer _ctrlWsServer;
  QList<MasterCtrlWsConnectionHandler*> _ctrlWsConnections;
};


#endif