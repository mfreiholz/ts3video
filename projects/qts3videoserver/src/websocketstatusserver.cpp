#include "websocketstatusserver.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include <Psapi.h>
#endif

#include <QCoreApplication>
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

WebSocketStatusServer::WebSocketStatusServer(const WebSocketStatusServer::Options &opts, TS3VideoServer *server) :
  QObject(server),
  _opts(opts),
  _server(server),
  _wsServer(new QWebSocketServer(QString(), QWebSocketServer::NonSecureMode, this)),
  _maxUpdateRate(2000)
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
  if (!_wsServer->listen(_opts.address, _opts.port)) {
    HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(6002).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listening for new client status web-socket connections (protocol=TCP; address=%1; port=%2)").arg(_opts.address.toString()).arg(_opts.port).toStdString());
  _lastUpdateTime.start();

  // DEV Run a timer to send periodical updates as long as we can't do it by events from server-object.
  auto broadcastTimer = new QTimer(this);
  broadcastTimer->setInterval(250);
  broadcastTimer->start();
  QObject::connect(broadcastTimer, &QTimer::timeout, this, &WebSocketStatusServer::broadcastAllInfo);

  return true;
}

void WebSocketStatusServer::broadcastAllInfo()
{
  if (_lastUpdateTime.elapsed() <= _maxUpdateRate)
    return;
  _lastUpdateTime.restart();

  auto root = getAllInfo().toObject();
  auto rootData = QJsonDocument(root).toJson(QJsonDocument::Compact);
  foreach(QWebSocket *socket, _sockets) {
    socket->sendTextMessage(rootData);
  }
}

void WebSocketStatusServer::onNewConnection()
{
  while (_wsServer->hasPendingConnections()) {
    auto socket = _wsServer->nextPendingConnection();
    QObject::connect(socket, &QWebSocket::textMessageReceived, this, &WebSocketStatusServer::onTextMessage);
    QObject::connect(socket, &QWebSocket::disconnected, this, &WebSocketStatusServer::onDisconnected);
    _sockets.append(socket);

    // Send all-info to new connected client.
    socket->sendTextMessage(QJsonDocument(getAllInfo().toObject()).toJson(QJsonDocument::Compact));
  }
}

void WebSocketStatusServer::onTextMessage(const QString &message)
{
  auto socket = qobject_cast<QWebSocket*>(sender());
  HL_TRACE(HL, QString("Incoming text message (message=%1)").arg(message).toStdString());
  //socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

void WebSocketStatusServer::onDisconnected()
{
  auto socket = qobject_cast<QWebSocket*>(sender());
  if (socket) {
    _sockets.removeAll(socket);
    socket->deleteLater();
  }
}

QJsonValue WebSocketStatusServer::getAllInfo() const
{
  QJsonObject root;
  root.insert("info", getAppInfo());
  root.insert("memory", getMemoryUsageInfo());
  root.insert("bandwidth", getBandwidthInfo());
  root.insert("clients", getClientsInfo());
  root.insert("channels", getChannelsInfo());
  root.insert("websockets", getWebSocketsInfo());
  return root;
}

QJsonValue WebSocketStatusServer::getAppInfo() const
{
  QJsonObject jsInfo;
  jsInfo.insert("appname", qApp->applicationName());
  jsInfo.insert("appversion", qApp->applicationVersion());
  jsInfo.insert("appdirectory", qApp->applicationDirPath());
  jsInfo.insert("appfilepath", qApp->applicationFilePath());
  jsInfo.insert("apppid", qApp->applicationPid());
  jsInfo.insert("organizationname", qApp->organizationName());
  jsInfo.insert("organizationdomain", qApp->organizationDomain());
  return jsInfo;
}

QJsonValue WebSocketStatusServer::getMemoryUsageInfo() const
{
  QJsonObject jsMemory;
#ifdef Q_OS_WIN
  PROCESS_MEMORY_COUNTERS_EX pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
  jsMemory.insert("pagefaultcount", (qint64)pmc.PageFaultCount);
  jsMemory.insert("peakworkingsetsize", (qint64)pmc.PeakWorkingSetSize);
  jsMemory.insert("workingsetsize", (qint64)pmc.WorkingSetSize);
  jsMemory.insert("quotapeakpagedpoolusage", (qint64)pmc.QuotaPeakPagedPoolUsage);
  jsMemory.insert("quotapagedpoolusage", (qint64)pmc.QuotaPagedPoolUsage);
  jsMemory.insert("quotapeaknonpagedpoolusage", (qint64)pmc.QuotaPeakNonPagedPoolUsage);
  jsMemory.insert("quotanonpagedpoolusage", (qint64)pmc.QuotaNonPagedPoolUsage);
  jsMemory.insert("pagefileusage", (qint64)pmc.PagefileUsage);
  jsMemory.insert("peakpagefileusage", (qint64)pmc.PeakPagefileUsage);
  jsMemory.insert("privateusage", (qint64)pmc.PrivateUsage);
#else
  jsMemory.insert("pagefaultcount", (qint64)-1);
  jsMemory.insert("peakworkingsetsize", (qint64)-1);
  jsMemory.insert("workingsetsize", (qint64)-1);
  jsMemory.insert("quotapeakpagedpoolusage", (qint64)-1);
  jsMemory.insert("quotapagedpoolusage", (qint64)-1);
  jsMemory.insert("quotapeaknonpagedpoolusage", (qint64)-1);
  jsMemory.insert("quotanonpagedpoolusage", (qint64)-1);
  jsMemory.insert("pagefileusage", (qint64)-1);
  jsMemory.insert("peakpagefileusage", (qint64)-1);
  jsMemory.insert("privateusage", (qint64)-1);
#endif
  return jsMemory;
}

QJsonValue WebSocketStatusServer::getBandwidthInfo() const
{
  return _server->_networkUsageMediaSocket.toQJsonObject();
}

QJsonValue WebSocketStatusServer::getClientsInfo() const
{
  QJsonArray clients;
  foreach(auto clientEntity, _server->_clients) {
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
  return clients;
}

QJsonValue WebSocketStatusServer::getChannelsInfo() const
{
  QJsonArray channels;
  foreach(auto channelEntity, _server->_channels) {
    auto jsChannel = channelEntity->toQJsonObject();
    QJsonArray jsParticipants;
    foreach(auto clientId, _server->_participants.value(channelEntity->id)) {
      jsParticipants.append(clientId);
    }
    jsChannel.insert("participants", jsParticipants);
    channels.append(jsChannel);
  }
  return channels;
}

QJsonValue WebSocketStatusServer::getWebSocketsInfo() const
{
  QJsonArray jsWsSockets;
  foreach(auto socket, _sockets) {
    QJsonObject o;
    o.insert("address", socket->peerAddress().toString());
    o.insert("port", socket->peerPort());
    jsWsSockets.append(o);
  }
  return jsWsSockets;
}
