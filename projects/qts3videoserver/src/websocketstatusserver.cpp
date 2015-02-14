#include "websocketstatusserver.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "humblelogging/api.h"

#include "ts3videoserver.h"
#include "clientconnectionhandler.h"
#include "cliententity.h"
#include "channelentity.h"

HUMBLE_LOGGER(HL, "server.status");

WebSocketStatusServer::WebSocketStatusServer(TS3VideoServer *server) :
  QObject(server),
  _server(server),
  _wsServer(new QWebSocketServer(QString(), QWebSocketServer::NonSecureMode, this))
{
  QObject::connect(_wsServer, &QWebSocketServer::newConnection, this, &WebSocketStatusServer::onNewConnection);
  QObject::connect(_wsServer, &QWebSocketServer::closed, this, &WebSocketStatusServer::closed);
}

WebSocketStatusServer::~WebSocketStatusServer()
{
  _wsServer->close();
  qDeleteAll(_sockets.begin(), _sockets.end());
}

bool WebSocketStatusServer::init()
{
  if (!_wsServer->listen(QHostAddress::Any, 6002)) {
    HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(6002).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listening for new client status web-socket connections (protocol=TCP; port=%1)").arg(6002).toStdString());
  return true;
}

void WebSocketStatusServer::onNewConnection()
{
  while (_wsServer->hasPendingConnections()) {
    auto socket = _wsServer->nextPendingConnection();
    QObject::connect(socket, &QWebSocket::textMessageReceived, this, &WebSocketStatusServer::onTextMessage);
    QObject::connect(socket, &QWebSocket::disconnected, this, &WebSocketStatusServer::onDisconnected);
    _sockets.append(socket);
  }
}

void WebSocketStatusServer::onTextMessage(const QString &message)
{
  auto socket = qobject_cast<QWebSocket*>(sender());
  HL_TRACE(HL, QString("Incoming text message (message=%1)").arg(message).toStdString());

  // Collect status information from TS3VideoServer.
  QJsonObject root;

  QJsonArray clients;
  foreach (auto clientEntity, _server->_clients) {
    auto jsClient = clientEntity->toQJsonObject();
    auto conn = _server->_connections.value(clientEntity->id);
    if (conn) {
      QJsonObject jsConn;
      jsConn.insert("address", "n/a");
      jsConn.insert("port", 0);
      jsConn.insert("mediaaddress", clientEntity->mediaAddress);
      jsConn.insert("mediaport", clientEntity->mediaPort);
      jsClient.insert("connection", jsConn);
    }
    clients.append(jsClient);
  }
  root.insert("clients", clients);

  QJsonArray channels;
  foreach (auto channelEntity, _server->_channels) {
    auto jsChannel = channelEntity->toQJsonObject();
    QJsonArray jsParticipants;
    foreach (auto clientId, _server->_participants.value(channelEntity->id)) {
      jsParticipants.append(clientId);
    }
    jsChannel.insert("participants", jsParticipants);
    channels.append(jsChannel);
  }
  root.insert("channels", channels);

  socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

void WebSocketStatusServer::onDisconnected()
{
  auto socket = qobject_cast<QWebSocket*>(sender());
  if (socket) {
    _sockets.removeAll(socket);
    socket->deleteLater();
  }
}