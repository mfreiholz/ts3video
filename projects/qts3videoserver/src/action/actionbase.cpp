#include "actionbase.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "humblelogging/api.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "ts3video.h"
#include "elws.h"
#include "jsonprotocolhelper.h"

#include "../virtualserver_p.h"
#include "../clientconnectionhandler.h"
#include "../servercliententity.h"
#include "../serverchannelentity.h"

HUMBLE_LOGGER(HL, "server.clientconnection.action");

///////////////////////////////////////////////////////////////////////

void ActionBase::broadcastNotificationToSiblingClients(const QString &action, const QJsonObject &params)
{
  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest(action, params));

  const auto pids = _req.server->getSiblingClientIds(_req.session->_clientEntity->id);
  foreach (const auto pid, pids) {
    const auto sess = _req.server->_connections.value(pid);
    if (sess && sess != _req.session) {
      const auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
}

///////////////////////////////////////////////////////////////////////

void AuthenticationAction::run()
{
  auto server = _req.server;
  auto &serverData = _req.server->d;
  auto conn = _req.connection;
  auto frame = _req.frame;
  auto action = _req.action;
  auto params = _req.params;

  auto clientVersion = params["version"].toString();
  auto clientSupportedServerVersions = params["supportedversions"].toString();
  auto username = params["username"].toString();
  auto password = params["password"].toString();
  auto videoEnabled = params["videoenabled"].toBool();

  // Compare client version against server version compatibility.
  if (!ELWS::isVersionSupported(clientVersion, IFVS_SOFTWARE_VERSION, clientSupportedServerVersions, IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS)) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(3, QString("Incompatible version (client=%1; server=%2)").arg(clientVersion).arg(IFVS_SOFTWARE_VERSION)));
    conn->sendResponse(res);
    QMetaObject::invokeMethod(conn, "disconnectFromHost", Qt::QueuedConnection);
    return;
  }

  // Authenticate.
  if (username.isEmpty() || (!server->options().password.isEmpty() && server->options().password != password)) {
    HL_WARN(HL, QString("Authentication failed by user (user=%1)").arg(username).toStdString());
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(4, QString("Authentication failed")));
    conn->sendResponse(res);
    QMetaObject::invokeMethod(conn, "disconnectFromHost", Qt::QueuedConnection);
    return;
  }

  _req.session->_authenticated = true;
  _req.session->_clientEntity->name = username;
  _req.session->_clientEntity->videoEnabled = videoEnabled;
  auto token = QString("%1-%2").arg(_req.session->_clientEntity->id).arg(QDateTime::currentDateTimeUtc().toString());
  server->_tokens.insert(token, _req.session->_clientEntity->id);

  // Send response.
  QJsonObject resData;
  resData["client"] = _req.session->_clientEntity->toQJsonObject();
  resData["authtoken"] = token;
  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(resData));
  conn->sendResponse(res);
  return;
}

///////////////////////////////////////////////////////////////////////

void GoodbyeAction::run()
{
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  _req.connection->sendResponse(res);
  _req.connection->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////

void HeartbeatAction::run()
{
  _req.session->_connectionTimeoutTimer.stop();
  _req.session->_connectionTimeoutTimer.start(20000);

  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  _req.connection->sendResponse(res);
}

///////////////////////////////////////////////////////////////////////

void EnableVideoAction::run()
{
  _req.session->_clientEntity->videoEnabled = true;

  // Respond to sender.
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  _req.session->_connection->sendResponse(res);

  // Broadcast to sibling clients.
  QJsonObject params;
  params["clientid"] = _req.session->_clientEntity->id;
  broadcastNotificationToSiblingClients("notify.clientvideoenabled", params);
}

///////////////////////////////////////////////////////////////////////

void DisableVideoAction::run()
{
  _req.session->_clientEntity->videoEnabled = false;

  // Respond to sender.
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  _req.session->_connection->sendResponse(res);

  // Broadcast to sibling clients.
  QJsonObject params;
  params["clientid"] = _req.session->_clientEntity->id;
  broadcastNotificationToSiblingClients("notify.clientvideodisabled", params);
}

///////////////////////////////////////////////////////////////////////

void JoinChannelAction::run()
{
  int channelId = 0;
  if (_req.action == "joinchannel") {
    channelId = _req.params["channelid"].toInt();
  }
  else if (_req.action == "joinchannelbyidentifier") {
    auto ident = _req.params["identifier"].toString();
    channelId = qHash(ident);
  }
  auto password = _req.params["password"].toString();


  // Validate parameters.
  if (channelId == 0 || (!_req.server->options().validChannels.isEmpty() && !_req.server->options().validChannels.contains(channelId))) {
    // Send error: Missing or invalid channel-id.
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
    _req.connection->sendResponse(res);
    return;
  }

  // Retrieve channel information.
  auto channelEntity = _req.server->_channels.value(channelId);

  // Verify password.
  if (channelEntity && !channelEntity->password.isEmpty() && channelEntity->password.compare(password) != 0) {
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(2, QString("Wrong password (channelid=%1)").arg(channelId)));
    _req.connection->sendResponse(res);
    return;
  }

  // Create channel, if it doesn't exists yet.
  if (!channelEntity) {
    channelEntity = new ServerChannelEntity();
    channelEntity->id = channelId;
    channelEntity->isPasswordProtected = !password.isEmpty();
    channelEntity->password = password;
    _req.server->_channels.insert(channelEntity->id, channelEntity);
  }

  // Associate the client's membership to the channel.
  _req.server->addClientToChannel(_req.session->_clientEntity->id, channelId);

  // Build response with information about the channel.
  auto participants = _req.server->_participants[channelEntity->id];
  QJsonObject params;
  params["channel"] = channelEntity->toQJsonObject();
  QJsonArray paramsParticipants;
  foreach(auto clientId, participants) {
    auto client = _req.server->_clients.value(clientId);
    if (client) {
      paramsParticipants.append(client->toQJsonObject());
    }
  }
  params["participants"] = paramsParticipants;

  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(params));
  _req.connection->sendResponse(res);

  // Notify participants about the new client.
  params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = _req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.clientjoinedchannel", params));
  foreach(auto clientId, participants) {
    auto sess = _req.server->_connections.value(clientId);
    if (sess && sess != _req.session) {
      auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
}

///////////////////////////////////////////////////////////////////////

void LeaveChannelAction::run()
{
  auto channelId = _req.params["channelid"].toInt();
  // Find channel.
  auto channelEntity = _req.server->_channels.value(channelId);
  if (!channelEntity) {
    // Send error: Invalid channel id.
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
    _req.connection->sendResponse(res);
    return;
  }
  // Send response.
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  _req.connection->sendResponse(res);

  // Notify participants.
  auto params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = _req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.clientleftchannel", params));
  auto participants = _req.server->_participants[channelEntity->id];
  foreach(auto clientId, participants) {
    auto sess = _req.server->_connections.value(clientId);
    if (sess && sess != _req.session) {
      auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
  // Leave channel.
  _req.server->removeClientFromChannel(_req.session->_clientEntity->id, channelId);
}

///////////////////////////////////////////////////////////////////////

void KickClientAction::run()
{
  const auto clientId = _req.params["clientid"].toInt();
  const auto bann = _req.params["bann"].toBool();

  // Find client session.
  auto sess = _req.server->_connections.value(clientId);
  if (!sess) {
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Unknown client")));
    sess->_connection->sendResponse(res);
    return;
  }

  // Kick client.
  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.kicked", QJsonObject()));
  const auto reply = sess->_connection->sendRequest(f);
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
  QMetaObject::invokeMethod(sess->_connection, "disconnectFromHost", Qt::QueuedConnection);

  // Update bann-list.
  if (bann) {
    _req.server->bann(QHostAddress(_req.session->_clientEntity->mediaAddress));
  }
}

///////////////////////////////////////////////////////////////////////