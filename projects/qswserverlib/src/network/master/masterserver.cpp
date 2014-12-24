#include "masterserver_p.h"
#include "QTcpSocket"
#include "QWebSocket"
#include "QHostAddress"
#include "humblelogging/api.h"
#include "mastermedianodeconnectionhandler.h"
#include "masterclientnodeconnectionhandler.h"
#include "masterctrlwsconnectionhandler.h"

HUMBLE_LOGGER(HL, "server.master");

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////

int randInt(int low, int high)
{
  return qrand() % ((high + 1) - low) + low;
}

///////////////////////////////////////////////////////////////////////////////
// MasterServer
///////////////////////////////////////////////////////////////////////////////

MasterServer::MasterServer(QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
}

MasterServer::~MasterServer()
{
  delete d;
}

bool MasterServer::startup(const Options &opts)
{
  d->_opts = opts;
  return d->startup();
}

bool MasterServer::shutdown()
{
  return d->shutdown();
}

MasterServer::ConnectionInfo MasterServer::getBestMediaServer() const
{
  const int index = randInt(0, d->_mediaNodeConnections.size());
  const MasterMediaNodeConnectionHandler *handler = d->_mediaNodeConnections.at(index);

  ConnectionInfo info;
  info.serverAddress = handler->getNodeStatus().externalStreamingAddress;
  info.serverPort = handler->getNodeStatus().externalStreamingPort;
  return info;
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MasterServer::Private::Private(MasterServer *owner) :
  QObject(0),
  _owner(owner),
  _ctrlWsServer("Master Management", QWebSocketServer::NonSecureMode)
{
  connect(&_mediaNodesServer, SIGNAL(newConnection()), SLOT(onNewMediaStreamingNodeConnection()));
  connect(&_clientNodesServer, SIGNAL(newConnection()), SLOT(onNewClientNodeConnection()));
  connect(&_ctrlWsServer, SIGNAL(newConnection()), SLOT(onNewCtrlWsConnection()));
}

MasterServer::Private::~Private()
{
}

bool MasterServer::Private::startup()
{
  // Listen for Media-Nodes.
  if (!_mediaNodesServer.listen(QHostAddress::Any, _opts.mediaNodesPort)) {
    HL_ERROR(HL, QString("Can not listen for media streaming nodes (port=%1)").arg(_opts.mediaNodesPort).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listen for media streaming nodes (port=%1)").arg(_opts.mediaNodesPort).toStdString());

  // Listen for Client-Nodes.
  if (!_clientNodesServer.listen(QHostAddress::Any, _opts.clientNodesPort)) {
    HL_ERROR(HL, QString("Can not listen for client nodes (port=%1)").arg(_opts.clientNodesPort).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listen for client nodes (port=%1)").arg(_opts.clientNodesPort).toStdString());

  // Listen for Ctrl-WS.
  if (!_ctrlWsServer.listen(QHostAddress::Any, _opts.ctrlWsPort)) {
    HL_ERROR(HL, QString("Can not listen for ctrl-ws (port=%1)").arg(_opts.ctrlWsPort).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listen for ctrl-ws (port=%1)").arg(_opts.ctrlWsPort).toStdString());

  return true;
}

bool MasterServer::Private::shutdown()
{
  if (_mediaNodesServer.isListening()) {
    _mediaNodesServer.close();
  }
  if (_clientNodesServer.isListening()) {
    _clientNodesServer.close();
  }
  if (_ctrlWsServer.isListening()) {
    _ctrlWsServer.close();
  }
  return true;
}

void MasterServer::Private::onNewMediaStreamingNodeConnection()
{
  while (_mediaNodesServer.hasPendingConnections()) {
    QTcpSocket *socket = _mediaNodesServer.nextPendingConnection();
    MasterMediaNodeConnectionHandler *handler = new MasterMediaNodeConnectionHandler(_owner, socket, this);
    connect(handler, SIGNAL(disconnected()), SLOT(onMediaStreamingNodeDisconnected()));
    _mediaNodeConnections.append(handler);
  }
}

void MasterServer::Private::onMediaStreamingNodeDisconnected()
{
  MasterMediaNodeConnectionHandler *handler = qobject_cast<MasterMediaNodeConnectionHandler*>(sender());
  _mediaNodeConnections.removeAll(handler);
  handler->deleteLater();
}

void MasterServer::Private::onNewClientNodeConnection()
{
  while (_clientNodesServer.hasPendingConnections()) {
    QTcpSocket *socket = _clientNodesServer.nextPendingConnection();
    MasterClientNodeConnectionHandler *handler = new MasterClientNodeConnectionHandler(_owner, socket, this);
    connect(handler, SIGNAL(disconnected()), SLOT(onClientNodeDisconnected()));
    _clientNodeConnections.append(handler);
  }
}

void MasterServer::Private::onClientNodeDisconnected()
{
  MasterClientNodeConnectionHandler *handler = qobject_cast<MasterClientNodeConnectionHandler*>(sender());
  _clientNodeConnections.removeAll(handler);
  handler->deleteLater();
}

void MasterServer::Private::onNewCtrlWsConnection()
{
  while (_ctrlWsServer.hasPendingConnections()) {
    QWebSocket *socket = _ctrlWsServer.nextPendingConnection();
    MasterCtrlWsConnectionHandler *handler = new MasterCtrlWsConnectionHandler(_owner, socket, this);
    connect(handler, SIGNAL(disconnected()), SLOT(onCtrlWsDisconnected()));
    _ctrlWsConnections.append(handler);
  }
}

void MasterServer::Private::onCtrlWsDisconnected()
{
  MasterCtrlWsConnectionHandler *handler = qobject_cast<MasterCtrlWsConnectionHandler*>(sender());
  _ctrlWsConnections.removeAll(handler);
  handler->deleteLater();
}