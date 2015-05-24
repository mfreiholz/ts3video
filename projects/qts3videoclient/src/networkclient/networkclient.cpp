#include "networkclient_p.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimerEvent>
#include <QDataStream>
#include <QImage>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>

#include "humblelogging/api.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "ts3video.h"
#include "timeutil.h"

#include "mediasocket.h"

HUMBLE_LOGGER(HL, "networkclient");

///////////////////////////////////////////////////////////////////////

NetworkClient::NetworkClient(QObject *parent) :
  QObject(parent),
  d(new NetworkClientPrivate(this))
{
  qRegisterMetaType<YuvFrameRefPtr>("YuvFrameRefPtr");
  qRegisterMetaType<NetworkUsageEntity>("NetworkUsageEntity");

  d->corSocket = new QCorConnection(this);
  connect(d->corSocket, &QCorConnection::stateChanged, this, &NetworkClient::onStateChanged);
  connect(d->corSocket, &QCorConnection::error, this, &NetworkClient::onError);
  connect(d->corSocket, &QCorConnection::newIncomingRequest, this, &NetworkClient::onNewIncomingRequest);

  d->heartbeatTimer.setInterval(10000);
  QObject::connect(&d->heartbeatTimer, &QTimer::timeout, this, &NetworkClient::sendHeartbeat);
}

NetworkClient::~NetworkClient()
{
  delete d->corSocket;
  delete d->mediaSocket;
}

void NetworkClient::setMediaEnabled(bool yesno)
{
  d->useMediaSocket = yesno;
}

const QAbstractSocket* NetworkClient::socket() const
{
  return d->corSocket->socket();
}

const ClientEntity& NetworkClient::clientEntity() const
{
  return d->clientEntity;
}

bool NetworkClient::isReadyForStreaming() const
{
  if (!d->mediaSocket || d->mediaSocket->state() != QAbstractSocket::ConnectedState) {
    return false;
  }
  if (!d->mediaSocket->isAuthenticated()) {
    return false;
  }
  return true;
}

bool NetworkClient::isAdmin() const
{
  return d->isAdmin;
}

bool NetworkClient::isSelf(const ClientEntity &ci) const
{
  return d->clientEntity.id == ci.id;
}

void NetworkClient::connectToHost(const QHostAddress &address, qint16 port)
{
  HL_DEBUG(HL, QString("Connect to server (address=%1; port=%2)").arg(address.toString()).arg(port).toStdString());
  d->corSocket->connectTo(address, port);
}

QCorReply* NetworkClient::auth(const QString &name, const QString &password, bool videoEnabled)
{
  HL_DEBUG(HL, QString("Authenticate with server (version=%1; supportedversions=%2; username=%3; password=<HIDDEN>)")
  .arg(IFVS_SOFTWARE_VERSION).arg(IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS).arg(name).toStdString());

  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }
  if (name.isEmpty()) {
    HL_ERROR(HL, QString("Empty username for authentication not allowed.").toStdString());
    return nullptr;
  }

  // Prepare authentication request.
  QJsonObject params;
  params["version"] = IFVS_SOFTWARE_VERSION;
  params["supportedversions"] = IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS;
  params["username"] = name;
  params["password"] = password;
  params["videoenabled"] = videoEnabled; ///< TODO REMOVE

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("auth", params));
  auto reply = d->corSocket->sendRequest(req);

  // Authentication response: Automatically connect media socket, if authentication was successful.
  connect(reply, &QCorReply::finished, [this, reply] () {
    int status = 0;
    QJsonObject params;
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params)) {
      return;
    } else if (status != 0) {
      return;
    }
    auto client = params["client"].toObject();
    auto authtoken = params["authtoken"].toString();
    // Get self client info from response.
    d->clientEntity.fromQJsonObject(client);
    // Create new media socket.
    if (d->useMediaSocket) {
      if (d->mediaSocket) {
        d->mediaSocket->close();
        delete d->mediaSocket;
      }
      d->mediaSocket = new MediaSocket(authtoken, this);
      d->mediaSocket->connectToHost(d->corSocket->socket()->peerAddress(), d->corSocket->socket()->peerPort());
      QObject::connect(d->mediaSocket, &MediaSocket::newVideoFrame, this, &NetworkClient::newVideoFrame);
      QObject::connect(d->mediaSocket, &MediaSocket::networkUsageUpdated, this, &NetworkClient::networkUsageUpdated);
    }
  });

  return reply;
}

QCorReply* NetworkClient::goodbye()
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }

  d->goodbye = true;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("goodbye", QJsonObject()));
  return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::joinChannel(int id, const QString &password)
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }

  QJsonObject params;
  params["channelid"] = id;
  params["password"] = password;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::joinChannelByIdentifier(const QString &ident, const QString &password)
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }

  QJsonObject params;
  params["identifier"] = ident;
  params["password"] = password;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannelbyidentifier", params));
  return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::enableVideoStream()
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }
  d->videoStreamingEnabled = true;

  QJsonObject params;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("clientenablevideo", params));
  return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::disableVideoStream()
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }
  d->videoStreamingEnabled = false;

  QJsonObject params;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("clientdisablevideo", params));
  return d->corSocket->sendRequest(req);
}

void NetworkClient::sendVideoFrame(const QImage &image)
{
  if (!d->mediaSocket) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return;
  }
  if (!d->videoStreamingEnabled) {
    return;
  }
  d->mediaSocket->sendVideoFrame(image, d->clientEntity.id);
}

QCorReply* NetworkClient::authAsAdmin(const QString &password)
{
  HL_DEBUG(HL, QString("Authorize as administrator").toStdString());
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }

  QJsonObject params;
  params["password"] = password;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("adminauth", params));
  auto reply = d->corSocket->sendRequest(req);

  connect(reply, &QCorReply::finished, [this, reply]() {
    int status = 0;
    QJsonObject params;
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params)) {
      return;
    }
    else if (status != 0) {
      return;
    }
    d->isAdmin = true;
  });

  return reply;
}

QCorReply* NetworkClient::kickClient(int clientId, bool ban)
{
  HL_DEBUG(HL, QString("Kick client (id=%1; ban=%2)").arg(clientId).arg(ban).toStdString());
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return nullptr;
  }

  QJsonObject params;
  params["clientid"] = clientId;
  params["ban"] = ban;

  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("kickclient", params));
  return d->corSocket->sendRequest(req);
}

void NetworkClient::sendHeartbeat()
{
  if (d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) {
    HL_ERROR(HL, QString("Connection is not established.").toStdString());
    return;
  }
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("heartbeat", QJsonObject()));
  auto reply = d->corSocket->sendRequest(req);
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
}

void NetworkClient::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      d->heartbeatTimer.start();
      emit connected();
      break;
    case QAbstractSocket::UnconnectedState:
      d->heartbeatTimer.stop();
      emit disconnected();
      break;
  }
}

void NetworkClient::onError(QAbstractSocket::SocketError err)
{
  if (d->goodbye) {
    d->goodbye = false;
    return;
  }
  emit error(err);
}

void NetworkClient::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  Q_ASSERT(!frame.isNull());
  HL_DEBUG(HL, QString("Incoming request (size=%1): %2").arg(frame->data().size()).arg(QString(frame->data())).toStdString());

  QString action;
  QJsonObject parameters;
  if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, parameters)) {
    // Invalid protocol.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(500, "Invalid protocol format."));
    d->corSocket->sendResponse(res);
    return;
  }

  if (action == "notify.mediaauthsuccess") {
    d->mediaSocket->setAuthenticated(true);
  }
  else if (action == "notify.clientjoinedchannel") {
    ChannelEntity channelEntity;
    channelEntity.fromQJsonObject(parameters["channel"].toObject());
    ClientEntity clientEntity;
    clientEntity.fromQJsonObject(parameters["client"].toObject());
    emit clientJoinedChannel(clientEntity, channelEntity);
  }
  else if (action == "notify.clientleftchannel") {
    ChannelEntity channelEntity;
    channelEntity.fromQJsonObject(parameters["channel"].toObject());
    ClientEntity clientEntity;
    clientEntity.fromQJsonObject(parameters["client"].toObject());
    emit clientLeftChannel(clientEntity, channelEntity);
  }
  else if (action == "notify.clientdisconnected") {
    ClientEntity clientEntity;
    clientEntity.fromQJsonObject(parameters["client"].toObject());
    emit clientDisconnected(clientEntity);
  }
  else if (action == "error") {
    auto errorCode = parameters["code"].toInt();
    auto errorMessage = parameters["message"].toString();
    emit serverError(errorCode, errorMessage);
  }

  // Response with error.
  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  d->corSocket->sendResponse(res);
}
