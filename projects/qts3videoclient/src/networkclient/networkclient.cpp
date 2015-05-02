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
  connect(d->corSocket, &QCorConnection::error, this, &NetworkClient::error);
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

void NetworkClient::connectToHost(const QHostAddress &address, qint16 port)
{
  Q_ASSERT(!address.isNull());
  Q_ASSERT(port > 0);
  d->corSocket->connectTo(address, port);
}

QCorReply* NetworkClient::auth(const QString &name, const QString &password, bool videoEnabled)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);

  QJsonObject params;
  params["version"] = IFVS_SOFTWARE_VERSION;
  params["username"] = name;
  params["password"] = password;
  params["videoenabled"] = videoEnabled;
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

QCorReply* NetworkClient::joinChannel(int id)
{
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);
  QJsonObject params;
  params["channelid"] = id;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->corSocket->sendRequest(req);
}

void NetworkClient::sendVideoFrame(const QImage &image)
{
  Q_ASSERT(d->mediaSocket);
  if (d->mediaSocket) {
    d->mediaSocket->sendVideoFrame(image, d->clientEntity.id);
  }
}

void NetworkClient::sendHeartbeat()
{
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);
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
