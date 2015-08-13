#include "actionbase.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTcpSocket>
#include <QtConcurrent>

#include "humblelogging/api.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "ts3util.h"

#include "ts3video.h"
#include "elws.h"
#include "jsonprotocolhelper.h"

#include "../virtualserver_p.h"
#include "../clientconnectionhandler.h"
#include "../servercliententity.h"
#include "../serverchannelentity.h"

HUMBLE_LOGGER(HL, "server.clientconnection.action");

///////////////////////////////////////////////////////////////////////

void ActionBase::sendDefaultOkResponse(const ActionData &req, const QJsonObject &params)
{
  // Create simple status=0 response frame.
  QCorFrame res;
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(params));

  // Send.
  req.connection->sendResponse(res);
}

void ActionBase::sendDefaultErrorResponse(const ActionData &req, int statusCode, const QString &message)
{
  // Create simple status=X response frame.
  QCorFrame res;
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponseError(statusCode, message));

  // Send.
  req.connection->sendResponse(res);
}

void ActionBase::broadcastNotificationToSiblingClients(const ActionData &req, const QString &action, const QJsonObject & params)
{
  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest(action, params));

  const auto pids = req.server->getSiblingClientIds(req.session->_clientEntity->id);
  foreach (const auto pid, pids) {
    const auto sess = req.server->_connections.value(pid);
    if (sess && sess != req.session) {
      const auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
}

void ActionBase::disconnectFromHostDelayed(const ActionData &req)
{
  QMetaObject::invokeMethod(req.connection, "disconnectFromHost", Qt::QueuedConnection);
}

///////////////////////////////////////////////////////////////////////

void AuthenticationAction::run(const ActionData &req)
{
  const auto clientVersion = req.params["version"].toString();
  const auto clientSupportedServerVersions = req.params["supportedversions"].toString();
  const auto username = req.params["username"].toString();
  const auto password = req.params["password"].toString();
  const auto videoEnabled = req.params["videoenabled"].toBool();

  if (true) {
    // Max number of connections (Connection limit).
    if (req.server->_connections.size() > req.server->options().connectionLimit) {
      HL_WARN(HL, QString("Server connection limit exceeded. (max=%1)").arg(req.server->options().connectionLimit).toStdString());
      sendDefaultErrorResponse(req, 1, "Server connection limit exceeded.");
      disconnectFromHostDelayed(req);
      return;
    }

    // Max bandwidth usage (Bandwidth limit).
    if (req.server->_networkUsageMediaSocket.bandwidthRead > req.server->options().bandwidthReadLimit || req.server->_networkUsageMediaSocket.bandwidthWrite > req.server->options().bandwidthWriteLimit) {
      HL_WARN(HL, QString("Server bandwidth limit exceeded.").toStdString());
      sendDefaultErrorResponse(req, 1, "Server bandwidth limit exceeded.");
      disconnectFromHostDelayed(req);
      return;
    }

    // Ban check.
    const auto peerAddress = req.connection->socket()->peerAddress();
    if (req.server->isBanned(peerAddress)) {
      HL_WARN(HL, QString("Banned user tried to connect. (address=%1)").arg(peerAddress.toString()).toStdString());
      sendDefaultErrorResponse(req, 1, "You are banned from the server.");
      disconnectFromHostDelayed(req);
      return;
    }
  }

  // Compare client version against server version compatibility.
  if (!ELWS::isVersionSupported(clientVersion, IFVS_SOFTWARE_VERSION, clientSupportedServerVersions, IFVS_SERVER_SUPPORTED_CLIENT_VERSIONS)) {
    HL_WARN(HL, QString("Incompatible version (client=%1; server=%2)").arg(clientVersion).arg(IFVS_SOFTWARE_VERSION).toStdString());
    sendDefaultErrorResponse(req, 3, QString("Incompatible version (client=%1; server=%2)").arg(clientVersion).arg(IFVS_SOFTWARE_VERSION));
    disconnectFromHostDelayed(req);
    return;
  }

  // Authenticate.
  if (username.isEmpty() || (!req.server->options().password.isEmpty() && req.server->options().password != password)) {
    HL_WARN(HL, QString("Authentication failed by user (user=%1)").arg(username).toStdString());
    sendDefaultErrorResponse(req, 4, QString("Authentication failed"));
    disconnectFromHostDelayed(req);
    return;
  }

  // TODO Ask TS3 Server, whether the client is connected.
  if (req.server->options().ts3Enabled)
  {
    auto f = QtConcurrent::run([&req]()
    {
      const auto& o = req.server->options();
      return TS3Util::isClientConnected(o.ts3Address, o.ts3Port, o.ts3LoginName, o.ts3LoginPassword, o.ts3VirtualServerPort, req.connection->socket()->peerAddress().toString());
    });
    if (!f.result())
    {
      HL_WARN(HL, QString("Authorization against TeamSpeak 3 failed (ip=%1)").arg(req.connection->socket()->peerAddress().toString()).toStdString());
      sendDefaultErrorResponse(req, 5, QString("Authentication failed (TeamSpeak 3 Bridge)"));
      disconnectFromHostDelayed(req);
      return;
    }
  }

  // Update self ClientEntity and generate auth-token for media socket.
  req.session->_authenticated = true;
  req.session->_clientEntity->name = username;
  req.session->_clientEntity->videoEnabled = videoEnabled;

  const auto token = QString("%1-%2").arg(req.session->_clientEntity->id).arg(QDateTime::currentDateTimeUtc().toString());
  req.server->_tokens.insert(token, req.session->_clientEntity->id);

  // Respond.
  QJsonObject params;
  params["client"] = req.session->_clientEntity->toQJsonObject();
  params["authtoken"] = token;
  sendDefaultOkResponse(req, params);
}

///////////////////////////////////////////////////////////////////////

void GoodbyeAction::run(const ActionData &req)
{
  sendDefaultOkResponse(req);
  disconnectFromHostDelayed(req);
}

///////////////////////////////////////////////////////////////////////

void HeartbeatAction::run(const ActionData &req)
{
  req.session->_connectionTimeoutTimer.stop();
  req.session->_connectionTimeoutTimer.start(20000);

  sendDefaultOkResponse(req);
}

///////////////////////////////////////////////////////////////////////

void EnableVideoAction::run(const ActionData &req)
{
  req.session->_clientEntity->videoEnabled = true;
  req.server->updateMediaRecipients();

  sendDefaultOkResponse(req);

  // Broadcast to sibling clients.
  QJsonObject params;
  params["client"] = req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients(req, "notify.clientvideoenabled", params);
}

///////////////////////////////////////////////////////////////////////

void DisableVideoAction::run(const ActionData &req)
{
  req.session->_clientEntity->videoEnabled = false;
  req.server->updateMediaRecipients();

  sendDefaultOkResponse(req);

  // Broadcast to sibling clients.
  QJsonObject params;
  params["client"] = req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients(req, "notify.clientvideodisabled", params);
}

///////////////////////////////////////////////////////////////////////

void EnableRemoteVideoAction::run(const ActionData &req)
{
  const auto clientId = req.params["clientid"].toInt();

  auto &set = req.session->_clientEntity->remoteVideoExcludes;
  if (set.remove(clientId)) {
    req.server->updateMediaRecipients();
  }

  sendDefaultOkResponse(req);
}

///////////////////////////////////////////////////////////////////////

void DisableRemoteVideoAction::run(const ActionData &req)
{
  const auto clientId = req.params["clientid"].toInt();

  auto &set = req.session->_clientEntity->remoteVideoExcludes;
  if (!set.contains(clientId)) {
    set.insert(clientId);
    req.server->updateMediaRecipients();
  }

  sendDefaultOkResponse(req);
}

///////////////////////////////////////////////////////////////////////

void JoinChannelAction::run(const ActionData &req)
{
  int channelId = 0;
  if (req.action == "joinchannel") {
    channelId = req.params["channelid"].toInt();
  }
  else if (req.action == "joinchannelbyidentifier") {
    auto ident = req.params["identifier"].toString();
    channelId = qHash(ident);
  }
  const auto password = req.params["password"].toString();

  // Validate parameters.
  if (channelId == 0 || (!req.server->options().validChannels.isEmpty() && !req.server->options().validChannels.contains(channelId))) {
    sendDefaultErrorResponse(req, 1, QString("Invalid channel id (channelid=%1)").arg(channelId));
    return;
  }

  // Retrieve channel information (It is not guaranteed that the channel already exists).
  auto channelEntity = req.server->_channels.value(channelId);

  // Verify password.
  if (channelEntity && !channelEntity->password.isEmpty() && channelEntity->password.compare(password) != 0) {
    sendDefaultErrorResponse(req, 2, QString("Wrong password (channelid=%1)").arg(channelId));
    return;
  }

  // Create channel, if it doesn't exists yet.
  if (!channelEntity) {
    channelEntity = new ServerChannelEntity();
    channelEntity->id = channelId;
    channelEntity->isPasswordProtected = !password.isEmpty();
    channelEntity->password = password;
    req.server->_channels.insert(channelEntity->id, channelEntity);
  }

  // Associate the client's membership to the channel.
  req.server->addClientToChannel(req.session->_clientEntity->id, channelId);
  req.server->updateMediaRecipients();

  // Build response with information about the channel.
  auto participants = req.server->_participants[channelEntity->id];
  QJsonObject params;
  params["channel"] = channelEntity->toQJsonObject();
  QJsonArray paramsParticipants;
  foreach(auto clientId, participants) {
    auto client = req.server->_clients.value(clientId);
    if (client) {
      paramsParticipants.append(client->toQJsonObject());
    }
  }
  params["participants"] = paramsParticipants;
  sendDefaultOkResponse(req, params);

  // Notify participants about the new client.
  params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients(req, "notify.clientjoinedchannel", params);
}

///////////////////////////////////////////////////////////////////////

void LeaveChannelAction::run(const ActionData &req)
{
  const auto channelId = req.params["channelid"].toInt();

  // Find channel.
  auto channelEntity = req.server->_channels.value(channelId);
  if (!channelEntity) {
    sendDefaultErrorResponse(req, 1, QString("Invalid channel id (channelid=%1)").arg(channelId));
    return;
  }

  sendDefaultOkResponse(req);

  // Notify participants.
  QJsonObject params;
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = req.session->_clientEntity->toQJsonObject();
  broadcastNotificationToSiblingClients(req, "notify.clientleftchannel", params);

  // Leave channel.
  req.server->removeClientFromChannel(req.session->_clientEntity->id, channelId);
  req.server->updateMediaRecipients();
}

///////////////////////////////////////////////////////////////////////

void AdminAuthAction::run(const ActionData &req)
{
  const auto password = req.params["password"].toString();

  // Verify password.
  const auto adminPassword = req.server->options().adminPassword;
  if (password.isEmpty() || adminPassword.isEmpty() || password.compare(adminPassword) != 0) {
    QCorFrame res;
    res.initResponse(*req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, "Wrong password"));
    req.connection->sendResponse(res);
    return;
  }

  req.session->_isAdmin = true;

  sendDefaultOkResponse(req);
}

///////////////////////////////////////////////////////////////////////

void KickClientAction::run(const ActionData &req)
{
  const auto clientId = req.params["clientid"].toInt();
  const auto ban = req.params["ban"].toBool();

  // Find client session.
  auto sess = req.server->_connections.value(clientId);
  if (!sess) {
    QCorFrame res;
    res.initResponse(*req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Unknown client")));
    sess->_connection->sendResponse(res);
    return;
  }

  // Kick client (+notify the client about it).
  QJsonObject params;
  params["client"] = req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.kicked", params));
  const auto reply = sess->_connection->sendRequest(f);
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
  QMetaObject::invokeMethod(sess->_connection, "disconnectFromHost", Qt::QueuedConnection);

  // Update ban-list.
  if (ban) {
    req.server->ban(QHostAddress(req.session->_clientEntity->mediaAddress));
  }

  sendDefaultOkResponse(req);
}

///////////////////////////////////////////////////////////////////////