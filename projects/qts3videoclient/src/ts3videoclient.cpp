#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimerEvent>
#include <QDataStream>

#include "medprotocol.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

using namespace UDP;

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

  if (action == "notify.mediaauthsuccess") {
    d->_mediaSocket->setAuthenticated(true);
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
  _authenticationTimerId(-1),
  _sendVideoFrameTimerId(-1)
{
  connect(this, &MediaSocket::stateChanged, this, &MediaSocket::onSocketStateChanged);
  connect(this, &MediaSocket::readyRead, this, &MediaSocket::onReadyRead);
}

MediaSocket::~MediaSocket()
{
  if (_authenticationTimerId != -1) {
    killTimer(_authenticationTimerId);
  }
  if (_sendVideoFrameTimerId != -1) {
    killTimer(_sendVideoFrameTimerId);
  }
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
  // TODO Remove demo code.
  if (_sendVideoFrameTimerId == -1) {
    _sendVideoFrameTimerId = startTimer(250);
  }
}

void MediaSocket::sendAuthTokenDatagram(const QString &token)
{
  qDebug() << QString("Send media auth token (token=%1; address=%2; port=%3)").arg(token).arg(peerAddress().toString()).arg(peerPort());

  UDP::AuthDatagram dgauth;
  dgauth.size = token.toUtf8().size();
  dgauth.data = new UDP::dg_byte_t[dgauth.size];
  memcpy(dgauth.data, token.toUtf8().data(), dgauth.size);

  QByteArray datagram;
  QDataStream out(&datagram, QIODevice::WriteOnly);
  out.setByteOrder(QDataStream::BigEndian);
  out << dgauth.magic;
  out << dgauth.type;
  out << dgauth.size;
  out.writeRawData((char*)dgauth.data, dgauth.size);
  writeDatagram(datagram, peerAddress(), peerPort());
}

void MediaSocket::sendVideoFrame(const QByteArray &frameData, quint64 frameId_, int senderId_)
{
  qDebug() << QString("Send video frame (size=%1; address=%2; port=%3)").arg(frameData.size()).arg(peerAddress().toString()).arg(peerPort());

  UDP::VideoFrameDatagram::dg_frame_id_t frameId = frameId_;
  UDP::VideoFrameDatagram::dg_sender_t senderId = senderId_;

  // Split frame into datagrams.
  VideoFrameDatagram **datagrams = 0;
  VideoFrameDatagram::dg_data_count_t datagramsLength;
  if (VideoFrameDatagram::split((dg_byte_t*)frameData.data(), frameData.size(), frameId, senderId, &datagrams, datagramsLength) != 0) {
    return; // Error.
  }

  // Send datagrams.
  for (auto i = 0; i < datagramsLength; ++i) {
    const auto &dgvideo = *datagrams[i];
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << dgvideo.magic;
    out << dgvideo.type;
    out << dgvideo.flags;
    out << dgvideo.sender;
    out << dgvideo.frameId;
    out << dgvideo.index;
    out << dgvideo.count;
    out << dgvideo.size;
    out.writeRawData((char*)dgvideo.data, dgvideo.size);
    writeDatagram(datagram, peerAddress(), peerPort());
  }
  VideoFrameDatagram::freeData(datagrams, datagramsLength);
}

void MediaSocket::timerEvent(QTimerEvent *ev)
{
  if (ev->timerId() == _authenticationTimerId) {
    sendAuthTokenDatagram(_token);
  }
  else if (ev->timerId() == _sendVideoFrameTimerId) {
    auto frameData = QByteArray();
    for (auto i = 0; i < 4096; ++i) {
      if (i == 0)
        frameData.append('A');
      else if (i == 4095)
        frameData.append('C');
      else
        frameData.append('B');
    }
    sendVideoFrame(frameData, 0, 0);
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

void MediaSocket::onReadyRead()
{
  while (hasPendingDatagrams()) {
    // Read datagram.
    QByteArray data;
    QHostAddress senderAddress;
    quint16 senderPort;
    data.resize(pendingDatagramSize());
    readDatagram(data.data(), data.size(), &senderAddress, &senderPort);

    qDebug() << QString("Incoming datagram (size=%1)").arg(data.size());

    QDataStream in(data);
    in.setByteOrder(QDataStream::BigEndian);

    // Check magic.
    UDP::Datagram dg;
    in >> dg.magic;
    if (dg.magic != UDP::Datagram::MAGIC) {
      qDebug() << QString("Invalid datagram (size=%1; data=%2)").arg(data.size()).arg(QString(data));
      continue;
    }

    // Handle by type.
    in >> dg.type;
    switch (dg.type) {
    }
  }
}