#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimerEvent>

#include "qcorconnection.h"
#include "qcorreply.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

///////////////////////////////////////////////////////////////////////

TS3VideoClient::TS3VideoClient(QObject *parent) :
  QObject(parent),
  d_ptr(new TS3VideoClientPrivate(this))
{
  Q_D(TS3VideoClient);
  d->_connection = new QCorConnection(this);
  connect(d->_connection, &QCorConnection::stateChanged, this, &TS3VideoClient::onStateChanged);
  connect(d->_connection, &QCorConnection::newIncomingRequest, this, &TS3VideoClient::onNewIncomingRequest);
}

TS3VideoClient::~TS3VideoClient()
{
  Q_D(TS3VideoClient);
  delete d->_connection;
  delete d->_mediaSocket;
}

void TS3VideoClient::connectToHost(const QHostAddress &address, qint16 port)
{
  Q_D(TS3VideoClient);
  d->_connection->connectTo(address, port);
}

QCorReply* TS3VideoClient::auth(const QString &name)
{
  Q_D(TS3VideoClient);
  QJsonObject params;
  params["version"] = 1;
  params["username"] = name;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("auth", params));
  auto reply = d->_connection->sendRequest(req);

  // Connect with media socket, if authentication was successful.
  connect(reply, &QCorReply::finished, [d, reply] () {
    int status = 0;
    QJsonObject params;
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params) || params["status"].toInt() != 0) {
      return;
    }
    // Create new media socket.
    if (d->_mediaSocket) {
      delete d->_mediaSocket;
    }
    d->_mediaSocket = new MediaSocket(params["authtoken"].toString(), d->q_ptr);
    d->_mediaSocket->connectToHost(d->_connection->socket()->peerAddress(), d->_connection->socket()->peerPort());
  });

  return reply;
}

QCorReply* TS3VideoClient::joinChannel()
{
  Q_D(TS3VideoClient);
  QJsonObject params;
  params["channelid"] = 1;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->_connection->sendRequest(req);
}

void TS3VideoClient::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      emit connected();
      break;
    case QAbstractSocket::UnconnectedState:
      emit disconnected();
      break;
  }
}

void TS3VideoClient::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  Q_D(TS3VideoClient);
  qDebug() << QString("Incoming request from server (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data()));
  
  QString action;
  QJsonObject parameters;
  if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, parameters)) {
    // Invalid protocol.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(500, "Invalid protocol format."));
    d->_connection->sendResponse(res);
    return;
  }

  if (action == "notify.clientjoinedchannel") {
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

  // Response with error.
  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
  d->_connection->sendResponse(res);
}

///////////////////////////////////////////////////////////////////////

TS3VideoClientPrivate::TS3VideoClientPrivate(TS3VideoClient *owner) :
  q_ptr(owner),
  _connection(nullptr),
  _mediaSocket(nullptr)
{
}

///////////////////////////////////////////////////////////////////////

MediaSocket::MediaSocket(const QString &token, QObject *parent) :
  QUdpSocket(parent),
  _authenticated(false),
  _token(token),
  _authenticationTimerId(-1)
{
  connect(this, &MediaSocket::stateChanged, this, &MediaSocket::onSocketStateChanged);
}

MediaSocket::~MediaSocket()
{
}

bool MediaSocket::authenticated() const
{
  return _authenticated;
}

void MediaSocket::setAuthenticated(bool yesno)
{
  _authenticated = yesno;
  if (_authenticationTimerId != -1) {
    killTimer(_authenticationTimerId);
    _authenticationTimerId = -1;
  }
}

void MediaSocket::timerEvent(QTimerEvent *ev)
{
  if (ev->timerId() == _authenticationTimerId) {
    qDebug() << QString("Send media auth token (token=%1; address=%2; port=%3)").arg(_token).arg(peerAddress().toString()).arg(peerPort());
    writeDatagram(_token.toUtf8(), peerAddress(), peerPort());
  }
}

void MediaSocket::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      if (_authenticationTimerId == -1) {
        _authenticationTimerId = startTimer(1000);
      }
      break;
    case QAbstractSocket::UnconnectedState:
      break;
  }
}
