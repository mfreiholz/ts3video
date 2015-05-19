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
#include "cliententity.h"
#include "../serverchannelentity.h"
#include "jsonprotocolhelper.h"

#include "../virtualserver.h"
#include "../virtualserver_p.h"
#include "../clientconnectionhandler.h"

HUMBLE_LOGGER(HL, "server.clientconnection.action");

///////////////////////////////////////////////////////////////////////

void AuthenticationAction::run()
{
  auto server = req.server;
  auto &serverData = req.server->d;
  auto conn = req.connection;
  auto frame = req.frame;
  auto action = req.action;
  auto params = req.params;

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

  req.session->_authenticated = true;
  req.session->_clientEntity->name = username;
  req.session->_clientEntity->videoEnabled = videoEnabled;
  auto token = QString("%1-%2").arg(req.session->_clientEntity->id).arg(QDateTime::currentDateTimeUtc().toString());
  server->_tokens.insert(token, req.session->_clientEntity->id);

  // Send response.
  QJsonObject resData;
  resData["client"] = req.session->_clientEntity->toQJsonObject();
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
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  req.connection->sendResponse(res);
  req.connection->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////

void HeartbeatAction::run()
{
  req.session->_connectionTimeoutTimer.stop();
  req.session->_connectionTimeoutTimer.start(20000);

  QCorFrame res;
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  req.connection->sendResponse(res);
}

///////////////////////////////////////////////////////////////////////

void JoinChannelAction::run()
{
  int channelId = 0;
  if (req.action == "joinchannel") {
    channelId = req.params["channelid"].toInt();
  }
  else if (req.action == "joinchannelbyidentifier") {
    auto ident = req.params["identifier"].toString();
    channelId = qHash(ident);
  }
  auto password = req.params["password"].toString();


  // Validate parameters.
  if (channelId == 0 || (!req.server->options().validChannels.isEmpty() && !req.server->options().validChannels.contains(channelId))) {
    // Send error: Missing or invalid channel-id.
    QCorFrame res;
    res.initResponse(*req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
    req.connection->sendResponse(res);
    return;
  }

  // Retrieve channel information.
  auto channelEntity = req.server->_channels.value(channelId);

  // Verify password.
  if (channelEntity && !channelEntity->password.isEmpty() && channelEntity->password.compare(password) != 0) {
    QCorFrame res;
    res.initResponse(*req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(2, QString("Wrong password (channelid=%1)").arg(channelId)));
    req.connection->sendResponse(res);
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

  QCorFrame res;
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(params));
  req.connection->sendResponse(res);

  // Notify participants about the new client.
  params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.clientjoinedchannel", params));
  foreach(auto clientId, participants) {
    auto sess = req.server->_connections.value(clientId);
    if (sess && sess != req.session) {
      auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
}

///////////////////////////////////////////////////////////////////////

void LeaveChannelAction::run()
{
  auto channelId = req.params["channelid"].toInt();
  // Find channel.
  auto channelEntity = req.server->_channels.value(channelId);
  if (!channelEntity) {
    // Send error: Invalid channel id.
    QCorFrame res;
    res.initResponse(*req.frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(1, QString("Invalid channel id (channelid=%1)").arg(channelId)));
    req.connection->sendResponse(res);
    return;
  }
  // Send response.
  QCorFrame res;
  res.initResponse(*req.frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  req.connection->sendResponse(res);

  // Notify participants.
  auto params = QJsonObject();
  params["channel"] = channelEntity->toQJsonObject();
  params["client"] = req.session->_clientEntity->toQJsonObject();

  QCorFrame f;
  f.setData(JsonProtocolHelper::createJsonRequest("notify.clientleftchannel", params));
  auto participants = req.server->_participants[channelEntity->id];
  foreach(auto clientId, participants) {
    auto sess = req.server->_connections.value(clientId);
    if (sess && sess != req.session) {
      auto reply = sess->_connection->sendRequest(f);
      QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
    }
  }
  // Leave channel.
  req.server->removeClientFromChannel(req.session->_clientEntity->id, channelId);
}

///////////////////////////////////////////////////////////////////////
