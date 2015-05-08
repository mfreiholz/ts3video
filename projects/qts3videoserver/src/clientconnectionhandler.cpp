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

#include "ts3video.h"
#include "elws.h"
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
  _networkUsage(),
  _networkUsageHelper(_networkUsage)
{
  _clientEntity = new ClientEntity();
  _clientEntity->id = ++_server->_nextClientId;
  _server->_clients.insert(_clientEntity->id, _clientEntity);

  _server->_connections.insert(_clientEntity->id, this);
  connect(_connection, &QCorConnection::stateChanged, this, &ClientConnectionHandler::onStateChanged);
  connect(_connection, &QCorConnection::newIncomingRequest, this, &ClientConnectionHandler::onNewIncomingRequest);
  onStateChanged(QAbstractSocket::ConnectedState);

  // Authentication timeout.
  // Close connection if it doesn't authentication within X seconds.
  QTimer::singleShot(5000, [this] () {
    if (!_connection)
      return;
    if (!_authenticated) {
      HL_WARN(HL, QString("Client did not authenticate within X seconds. Dropping connection...").toStdString());
      QCorFrame req;
      QJsonObject params;
      params["code"] = 1;
      params["message"] = "Authentication timed out.";
      req.setData(JsonProtocolHelper::createJsonRequest("error", params));
      auto reply = _connection->sendRequest(req);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
      QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
    }
  });

  // Connection timeout.
  _connectionTimeoutTimer.start(20000);
  QObject::connect(&_connectionTimeoutTimer, &QTimer::timeout, [this] () {
    if (!_connection)
      return;
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
  connect(statisticTimer, &QTimer::timeout, [this] () {
    _networkUsageHelper.recalculate();
    emit networkUsageUpdated(_networkUsage);
  });

  // Max number of connections (Connection limit).
  if (_server->_connections.size() > _server->_opts.connectionLimit) {
    HL_WARN(HL, QString("Server connection limit exceeded. (max=%1)").arg(_server->_opts.connectionLimit).toStdString());
    QCorFrame req;
    QJsonObject params;
    params["code"] = 1;
    params["message"] = "Server connection limit exceeded.";
    req.setData(JsonProtocolHelper::createJsonRequest("error", params));
    auto reply = _connection->sendRequest(req);
    connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
  }

  // Max bandwidth usage (Bandwidth limit).
  if (_server->_networkUsageMediaSocket.bandwidthRead > _server->_opts.bandwidthReadLimit || _server->_networkUsageMediaSocket.bandwidthWrite > _server->_opts.bandwidthWriteLimit) {
    HL_WARN(HL, QString("Server bandwidth limit exceeded.").toStdString());
    QCorFrame req;
    QJsonObject params;
    params["code"] = 1;
    params["message"] = "Server bandwidth limit exceeded.";
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
  _clientEntity = nullptr;
  _connection = nullptr;
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
  _networkUsage.bytesRead += frame->data().size(); // TODO Not correct, we need to get values from QCORLIB to include bytes of cor_frame (same for write).

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
    auto version = params["version"].toString();
    auto username = params["username"].toString();
    auto password = params["password"].toString();
    auto videoEnabled = params["videoenabled"].toBool();
    // Compare client version against server version compatibility.
    if (!ELWS::isVersionSupported(version, IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS)) {
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(3, QString("Incompatible version (client=%1; server=%2)").arg(version).arg(IFVS_SOFTWARE_VERSION)));
      _connection->sendResponse(res);
      QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
      return;
    }
    // Authenticate.
    if (username.isEmpty() || (!_server->_opts.password.isEmpty() && _server->_opts.password != password)) {
      HL_WARN(HL, QString("Authentication failed by user (user=%1)").arg(username).toStdString());
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(JsonProtocolHelper::createJsonResponseError(4, QString("Authentication failed")));
      _connection->sendResponse(res);
      QMetaObject::invokeMethod(_connection, "disconnectFromHost", Qt::QueuedConnection);
      return;
    }
    _authenticated = true;
    _clientEntity->name = username;
    _clientEntity->videoEnabled = videoEnabled;
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
    _connectionTimeoutTimer.stop();
    _connectionTimeoutTimer.start(20000);
    return;
  }
  else if (action == "joinchannel" || action == "joinchannelbyidentifier") {
    int channelId = 0;
    if (action == "joinchannel") {
      channelId = params["channelid"].toString().toLongLong();
    }
    else if (action == "joinchannelbyidentifier") {
      auto ident = params["identifier"].toString();
      channelId = qHash(ident);
    }
    // Validate parameters.
    if (channelId == 0 || (!_server->_opts.validChannels.isEmpty() && !_server->_opts.validChannels.contains(channelId))) {
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
