#include "actionbase.h"

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
#include "jsonprotocolhelper.h"

#include "../virtualserver_p.h"
#include "../clientconnectionhandler.h"
#include "../servercliententity.h"
#include "../serverchannelentity.h"

HUMBLE_LOGGER(HL, "server.clientconnection.action");

///////////////////////////////////////////////////////////////////////

void ActionBase::sendDefaultOkResponse(const QJsonObject &params)
{
  // Create simple status=0 response frame.
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(params));

  // Send.
  _req.connection->sendResponse(res);
}

void ActionBase::sendDefaultErrorResponse(int statusCode, const QString &message)
{
  // Create simple status=X response frame.
  QCorFrame res;
  res.initResponse(*_req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponseError(statusCode, message));

  // Send.
  _req.connection->sendResponse(res);
}

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

void ActionBase::disconnectFromHostDelayed()
{
  QMetaObject::invokeMethod(_req.connection, "disconnectFromHost", Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////

void AuthenticationAction::run()
{
  const auto clientVersion = _req.params["version"].toString();
  const auto clientSupportedServerVersions = _req.params["supportedversions"].toString();
  const auto username = _req.params["username"].toString();
  const auto password = _req.params["password"].toString();
  const auto videoEnabled = _req.params["videoenabled"].toBool();

  if (true) {
    // Max number of connections (Connection limit).
    if (_req.server->_connections.size() > _req.server->options().connectionLimit) {
      HL_WARN(HL, QString("Server connection limit exceeded. (max=%1)").arg(_req.server->options().connectionLimit).toStdString());
      sendDefaultErrorResponse(1, "Server connection limit exceeded.");
      disconnectFromHostDelayed();
      return;
    }

    // Max bandwidth usage (Bandwidth limit).
    if (_req.server->_networkUsageMediaSocket.bandwidthRead > _req.server->options().bandwidthReadLimit || _req.server->_networkUsageMediaSocket.bandwidthWrite > _req.server->options().bandwidthWriteLimit) {
      HL_WARN(HL, QString("Server bandwidth limit exceeded.").toStdString());
      sendDefaultErrorResponse(1, "Server bandwidth limit exceeded.");
      disconnectFromHostDelayed();
      return;
    }

    // Ban check.
    const auto peerAddress = _req.connection->socket()->peerAddress();
    if (_req.server->isBanned(peerAddress)) {
      HL_WARN(HL, QString("Banned user tried to connect. (address=%1)").arg(peerAddress.toString()).toStdString());
      sendDefaultErrorResponse(1, "You are banned from the server.");
      disconnectFromHostDelayed();
      return;
    }
  }

  // Compare client version against server version compatibility.
  if (!ELWS::isVersionSupported(clientVersion, IFVS_SOFTWARE_VERSION, clientSupportedServerVersions, IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS)) {
    HL_WARN(HL, QString("Incompatible version (client=%1; server=%2)").arg(clientVersion).arg(IFVS_SOFTWARE_VERSION).toStdString());
    sendDefaultErrorResponse(3, QString("Incompatible version (client=%1; server=%2)").arg(clientVersion).arg(IFVS_SOFTWARE_VERSION));
    disconnectFromHostDelayed();
    return;
  }

  // Authenticate.
  if (username.isEmpty() || (!_req.server->options().password.isEmpty() && _req.server->options().password != password)) {
    HL_WARN(HL, QString("Authentication failed by user (user=%1)").arg(username).toStdString());
    sendDefaultErrorResponse(4, QString("Authentication failed"));
    disconnectFromHostDelayed();
    return;
  }

  // Update self ClientEntity and generate auth-token for media socket.
  _req.session->_authenticated = true;
  _req.session->_clientEntity->name = username;
  _req.session->_clientEntity->videoEnabled = videoEnabled;

  const auto token = QString("%1-%2").arg(_req.session->_clientEntity->id).arg(QDateTime::currentDateTimeUtc().toString());
  _req.server->_tokens.insert(token, _req.session->_clientEntity->id);

  // Respond.
  QJsonObject params;
  params["client"] = _req.session->_clientEntity->toQJsonObject();
  params["authtoken"] = token;
  sendDefaultOkResponse(params);
}

///////////////////////////////////////////////////////////////////////

void GoodbyeAction::run()
{
  sendDefaultOkResponse();
  disconnectFromHostDelayed();
}

///////////////////////////////////////////////////////////////////////

void HeartbeatAction::run()
{
  _req.session->_connectionTimeoutTimer.stop();
  _req.session->_connectionTimeoutTimer.start(20000);

  sendDefaultOkResponse();
}

///////////////////////////////////////////////////////////////////////

void EnableVideoAction::run()
{
  _req.session->_clientEntity->videoEnabled = true;
  _req.server->updateMediaRecipients();

  sendDefaultOkResponse();

  // Broadcast to sibling clients.
  QJsonObject params;
  params["client"] = _req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients("notify.clientvideoenabled", params);
}

///////////////////////////////////////////////////////////////////////

void DisableVideoAction::run()
{
  _req.session->_clientEntity->videoEnabled = false;
  _req.server->updateMediaRecipients();

  sendDefaultOkResponse();

  // Broadcast to sibling clients.
  QJsonObject params;
  params["client"] = _req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients("notify.clientvideodisabled", params);
}

///////////////////////////////////////////////////////////////////////

void EnableRemoteVideoAction::run()
{
  const auto clientId = _req.params["clientid"].toInt();

  auto &set = _req.session->_clientEntity->remoteVideoExcludes;
  if (set.remove(clientId)) {
    _req.server->updateMediaRecipients();
  }

  sendDefaultOkResponse();
}

///////////////////////////////////////////////////////////////////////

void DisableRemoteVideoAction::run()
{
  const auto clientId = _req.params["clientid"].toInt();

  auto &set = _req.session->_clientEntity->remoteVideoExcludes;
  if (!set.contains(clientId)) {
    set.insert(clientId);
    _req.server->updateMediaRecipients();
  }

  sendDefaultOkResponse();
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
  const auto password = _req.params["password"].toString();

  // Validate parameters.
  if (channelId == 0 || (!_req.server->options().validChannels.isEmpty() && !_req.server->options().validChannels.contains(channelId))) {
    sendDefaultErrorResponse(1, QString("Invalid channel id (channelid=%1)").arg(channelId));
    return;
  }

  // Retrieve channel information (It is not guaranteed that the channel already exists).
  auto channelEntity = _req.server->_channels.value(channelId);

  // Verify password.
  if (channelEntity && !channelEntity->password.isEmpty() && channelEntity->password.compare(password) != 0) {
    sendDefaultErrorResponse(2, QString("Wrong password (channelid=%1)").arg(channelId));
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
  _req.server->updateMediaRecipients();

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
  sendDefaultOkResponse(params);

  // Notify participants about the new client.
  params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = _req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients("notify.clientjoinedchannel", params);
}

///////////////////////////////////////////////////////////////////////

void LeaveChannelAction::run()
{
  const auto channelId = _req.params["channelid"].toInt();

  // Find channel.
  auto channelEntity = _req.server->_channels.value(channelId);
  if (!channelEntity) {
    sendDefaultErrorResponse(1, QString("Invalid channel id (channelid=%1)").arg(channelId));
    return;
  }

  sendDefaultOkResponse();

  // Notify participants.
  QJsonObject params;
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = _req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients("notify.clientleftchannel", params);

  // Leave channel.
  _req.server->removeClientFromChannel(_req.session->_clientEntity->id, channelId);
  _req.server->updateMediaRecipients();
}

///////////////////////////////////////////////////////////////////////

void AdminAuthAction::run()
{
  const auto password = _req.params["password"].toString();

  // Verify password.
  const auto adminPassword = _req.server->options().adminPassword;
  if (password.isEmpty() || adminPassword.isEmpty() || password.compare(adminPassword) != 0) {
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, "Wrong password"));
    _req.connection->sendResponse(res);
    return;
  }

  _req.session->_isAdmin = true;

  sendDefaultOkResponse();
}

///////////////////////////////////////////////////////////////////////

void KickClientAction::run()
{
  const auto clientId = _req.params["clientid"].toInt();
  const auto ban = _req.params["ban"].toBool();

  // Find client session.
  auto sess = _req.server->_connections.value(clientId);
  if (!sess) {
    QCorFrame res;
    res.initResponse(*_req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Unknown client")));
    sess->_connection->sendResponse(res);
    return;
  }

  // Kick client (+notify the client about it).
  QJsonObject params;
  params["client"] = _req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.kicked", params));
  const auto reply = sess->_connection->sendRequest(f);
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
  QMetaObject::invokeMethod(sess->_connection, "disconnectFromHost", Qt::QueuedConnection);

  // Update ban-list.
  if (ban) {
    _req.server->ban(QHostAddress(_req.session->_clientEntity->mediaAddress));
  }

  sendDefaultOkResponse();
}

///////////////////////////////////////////////////////////////////////