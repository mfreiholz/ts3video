#include "clientconnectionhandler.h"

#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTcpSocket>

#include "humblelogging/api.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

#include "ts3videoserver.h"

HUMBLE_LOGGER(HL, "server.clientconnection");

///////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(TS3VideoServer *server, QCorConnection *connection, QObject *parent) :
  QObject(parent),
  _server(server),
  _connection(connection),
  _authenticated(false),
  _clientEntity(nullptr),
  _bytesRead(0),
  _bytesWritten(0),
  _bytesReadSince(0),
  _bytesWrittenSince(0)
{
  _clientEntity = new ClientEntity();
  _clientEntity->id = ++_server->_nextClientId;
  _server->_clients.insert(_clientEntity->id, _clientEntity);
  
  _server->_connections.insert(_clientEntity->id, this);
  connect(_connection, &QCorConnection::stateChanged, this, &ClientConnectionHandler::onStateChanged);
  connect(_connection, &QCorConnection::newIncomingRequest, this, &ClientConnectionHandler::onNewIncomingRequest);
  onStateChanged(QAbstractSocket::ConnectedState);

  // Initialize connection timeout timer.
  _timeoutTimer.start(20000);
  QObject::connect(&_timeoutTimer, &QTimer::timeout, [this]() {
    HL_WARN(HL, QString("Client connection timed out. No heartbeat since 20 seconds.").toStdString());
    QCorFrame req;
    QJsonObject params;
    params["code"] = 1;
    params["message"] = "Connection timed out.";
    req.setData(JsonProtocolHelper::createJsonRequest("error", params));
    auto reply = _connection->sendRequest(req);
    QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
  });

  // Prepare statistics.
  auto statisticTimer = new QTimer(this);
  statisticTimer->setInterval(1500);
  statisticTimer->start();
  connect(statisticTimer, &QTimer::timeout, this, &ClientConnectionHandler::updateStatistics);
  _bytesReadTime.start();
  _bytesWrittenTime.start();

  // Handle: Max number of connections (Connection limit).
  if (_server->_connections.size() > _server->_opts.connectionLimit) {
    HL_WARN(HL, QString("Maximum allowed connections exceeded. (max=%1)").arg(_server->_opts.connectionLimit).toStdString());
    QCorFrame req;
    QJsonObject params;
    params["code"] = 1;
    params["message"] = "Maximum allowed connections exceeded.";
    req.setData(JsonProtocolHelper::createJsonRequest("error", params));
    auto reply = _connection->sendRequest(req);
    connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
  }
}

ClientConnectionHandler::~ClientConnectionHandler()
{
  _server->removeClientFromChannels(_clientEntity->id);
  _server->_clients.remove(_clientEntity->id);
  _server->_connections.remove(_clientEntity->id);
  delete _clientEntity;
  delete _connection;
  _server = nullptr;
}

void ClientConnectionHandler::sendMediaAuthSuccessNotify()
{
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("notify.mediaauthsuccess", QJsonObject()));
  auto reply = _connection->sendRequest(req);
  connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
}

void ClientConnectionHandler::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState: {
      HL_INFO(HL, QString("New client connection (addr=%1; port=%2; clientid=%3)").arg(_connection->socket()->peerAddress().toString()).arg(_connection->socket()->peerPort()).arg(_clientEntity->id).toStdString());
      break;
    }
    case QAbstractSocket::UnconnectedState: {
      HL_INFO(HL, QString("Client disconnected (addr=%1; port=%2; clientid=%3)").arg(_connection->socket()->peerAddress().toString()).arg(_connection->socket()->peerPort()).arg(_clientEntity->id).toStdString());
      // Notify sibling clients about the disconnect.
      QJsonObject params;
      params["client"] = _clientEntity->toQJsonObject();
      QCorFrame req;
      req.setData(JsonProtocolHelper::createJsonRequest("notify.clientdisconnected", params));
      auto clientConns = _server->_connections.values();
      foreach(auto clientConn, clientConns) {
        if (clientConn && clientConn != this) {
          auto reply = clientConn->_connection->sendRequest(req);
          if (reply) connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
        }
      }
      // Delete itself.
      deleteLater();
      break;
    }
  }
}

void ClientConnectionHandler::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  HL_TRACE(HL, QString("New incoming request (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data())).toStdString());

  QString action;
  QJsonObject params;
  if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, params)) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(500, QString("Invalid protocol format")));
    _connection->sendResponse(res);
    return;
  }

  if (action == "auth") {
    auto version = params["version"].toInt();
    auto username = params["username"].toString();
    // Compare client version against server version compatibility.
    if (version != TS3VIDEOSERVER_VERSION) {
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(3, QString("Incompatible version (client=%1; server=%2)").arg(version).arg(TS3VIDEOSERVER_VERSION)));
      _connection->sendResponse(res);
      return;
    }
    // Authenticate.
    if (username.isEmpty()) {
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(4, QString("Authentication failed")));
      _connection->sendResponse(res);
      return;
    }
    _authenticated = true;
    _clientEntity->name = username;
    auto token = QString("%1-%2").arg(_clientEntity->id).arg(QDateTime::currentDateTimeUtc().toString());
    _server->_tokens.insert(token, _clientEntity->id);
    // Send response.
    QJsonObject resData;
    resData["client"] = _clientEntity->toQJsonObject();
    resData["authtoken"] = token;
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponse(resData));
    _connection->sendResponse(res);
    return;
  }
  else if (action == "goodbye") {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
    _connection->sendResponse(res);
    _connection->disconnectFromHost();
    return;
  }

  // The client needs to be authenticated before he can request any other actions.
  // Close connection, if the client tries anything else.
  if (!_authenticated) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(4, QString("Authentication failed")));
    _connection->sendResponse(res);
    _connection->disconnectFromHost();
    return;
  }

  if (action == "heartbeat") {
    // Send response.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
    _connection->sendResponse(res);
    _timeoutTimer.stop();
    _timeoutTimer.start(20000);
    return;
  }
  else if (action == "joinchannel") {
    auto channelId = params["channelid"].toInt();
    if (channelId <= 0) {
      // Send error: Missing channel id.
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
      _connection->sendResponse(res);
      return;
    }
    // Join channel.
    auto channelEntity = _server->addClientToChannel(_clientEntity->id, channelId);
    // Send response.
    auto participants = _server->_participants[channelEntity->id];
    QJsonObject params;
    params["channel"] = channelEntity->toQJsonObject();
    QJsonArray paramsParticipants;
    foreach (auto clientId, participants) {
      auto client = _server->_clients.value(clientId);
      if (client) {
        paramsParticipants.append(client->toQJsonObject());
      }
    }
    params["participants"] = paramsParticipants;
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponse(params));
    _connection->sendResponse(res);
    // Notify participants about the new client.
    params = QJsonObject();
    params["channel"] = channelEntity->toQJsonObject();
    params["client"] = _clientEntity->toQJsonObject();
    QCorFrame req;
    req.setData(JsonProtocolHelper::createJsonRequest("notify.clientjoinedchannel", params));
    foreach (auto clientId, participants) {
      auto conn = _server->_connections.value(clientId);
      if (conn && conn != this) {
        auto reply = conn->_connection->sendRequest(req);
        connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
      }
    }
    return;
  }
  else if (action == "leavechannel") {
    auto channelId = params["channelid"].toInt();
    // Find channel.
    auto channelEntity = _server->_channels.value(channelId);
    if (!channelEntity) {
      // Send error: Invalid channel id.
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
      _connection->sendResponse(res);
      return;
    }
    // Send response.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
    _connection->sendResponse(res);
    // Notify participants.
    params = QJsonObject();
    params["channel"] = channelEntity->toQJsonObject();
    params["client"] = _clientEntity->toQJsonObject();
    QCorFrame req;
    req.setData(JsonProtocolHelper::createJsonRequest("notify.clientleftchannel", params));
    auto participants = _server->_participants[channelEntity->id];
    foreach(auto clientId, participants) {
      auto conn = _server->_connections.value(clientId);
      if (conn && conn != this) {
        auto reply = conn->_connection->sendRequest(req);
        connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
      }
    }
    // Leave channel.
    _server->removeClientFromChannel(_clientEntity->id, channelId);
    return;
  }

  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(JsonProtocolHelper::createJsonResponseError(4, QString("Unknown action.")));
  _connection->sendResponse(res);
  return;
}

void ClientConnectionHandler::updateStatistics()
{
  // Calculate READ transfer rate.
  double readRate = 0.0;
  auto elapsedms = _bytesReadTime.elapsed();
  if (elapsedms > 0) {
    auto diff = _bytesRead - _bytesReadSince;
    if (diff > 0) {
      readRate = ((double)diff / elapsedms) * 1000;
    }
    _bytesReadSince = _bytesRead;
    _bytesReadTime.restart();
  }

  // Calculate WRITE transfer rate.
  double writeRate = 0.0;
  elapsedms = _bytesWrittenTime.elapsed();
  if (elapsedms > 0) {
    auto diff = _bytesWritten - _bytesWrittenSince;
    if (diff > 0) {
      writeRate = ((double)diff / elapsedms) * 1000;
    }
    _bytesWrittenSince = _bytesWritten;
    _bytesWrittenTime.restart();
  }
}