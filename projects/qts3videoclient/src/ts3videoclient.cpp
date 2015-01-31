#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimerEvent>
#include <QDataStream>
#include <QImage>

#include "medprotocol.h"

#include "qcorconnection.h"
#include "qcorreply.h"

/*
  Notes
  =====
  - Reading UDP datagrams in same thread may cause the GUI to hang, if there are a lot of clients?
*/

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
  if (d->_encodingThread) {
    d->_encodingThread->stop();
    d->_encodingThread->wait();
    delete d->_encodingThread;
  }
}

const ClientEntity& TS3VideoClient::clientEntity() const
{
  Q_D(const TS3VideoClient);
  return d->_clientEntity;
}

bool TS3VideoClient::isReadyForStreaming() const
{
  Q_D(const TS3VideoClient);
  if (d->_mediaSocket->state() != QAbstractSocket::ConnectedState) {
    return false;
  }
  if (!d->_mediaSocket->isAuthenticated()) {
    return false;
  }
  return true;
}

void TS3VideoClient::connectToHost(const QHostAddress &address, qint16 port)
{
  Q_D(TS3VideoClient);
  Q_ASSERT(!address.isNull());
  Q_ASSERT(port > 0);
  d->_connection->connectTo(address, port);
}

QCorReply* TS3VideoClient::auth(const QString &name)
{
  Q_D(TS3VideoClient);
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(d->_connection->socket()->state() == QAbstractSocket::ConnectedState);

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
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params)) {
      return;
    }
    else if (params["status"].toInt() != 0) {
      return;
    }
    // Get self client info from response.
    d->_clientEntity.fromQJsonObject(params["client"].toObject());
    // Create new media socket.
    if (d->_mediaSocket) {
      d->_mediaSocket->close();
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
  Q_ASSERT(d->_connection->socket()->state() == QAbstractSocket::ConnectedState);
  QJsonObject params;
  params["channelid"] = 1;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->_connection->sendRequest(req);
}

void TS3VideoClient::sendVideoFrame(const QImage &image)
{
  Q_D(TS3VideoClient);
  if (d->_encodingThread) {
    d->_encodingThread->enqueue(image);
  }
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
  Q_ASSERT(!frame.isNull());
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
    if (d->_encodingThread) {
      delete d->_encodingThread;
    }
    d->_encodingThread = new VideoEncodingThread(this);
    d->_encodingThread->start();
    static quint64 __frameid = 1;
    connect(d->_encodingThread, &VideoEncodingThread::newEncodedFrame, [d] (const QByteArray &frame) {
      d->_mediaSocket->sendVideoFrame(frame, __frameid++, d->_clientEntity.id);
    });
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
  _mediaSocket(nullptr),
  _clientEntity(),
  _encodingThread(nullptr)
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
  connect(this, &MediaSocket::readyRead, this, &MediaSocket::onReadyRead);
}

MediaSocket::~MediaSocket()
{
  if (_authenticationTimerId != -1) {
    killTimer(_authenticationTimerId);
  }
}

bool MediaSocket::isAuthenticated() const
{
  return _authenticated;
}

void MediaSocket::setAuthenticated(bool yesno)
{
  _authenticated = yesno;
  if (_authenticated && _authenticationTimerId != -1) {
    killTimer(_authenticationTimerId);
    _authenticationTimerId = -1;
  }
}

void MediaSocket::sendAuthTokenDatagram(const QString &token)
{
  Q_ASSERT(!token.isEmpty());
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

void MediaSocket::sendVideoFrame(const QByteArray &frame_, quint64 frameId_, quint32 senderId_)
{
  Q_ASSERT(frame_.isEmpty() == false);
  Q_ASSERT(frameId_ != 0);
  //qDebug() << QString("Send video frame (size=%1; address=%2; port=%3)").arg(frameData.size()).arg(peerAddress().toString()).arg(peerPort());
  
  if (state() == QAbstractSocket::ConnectedState) {
    return;
  }

  UDP::VideoFrameDatagram::dg_frame_id_t frameId = frameId_;
  UDP::VideoFrameDatagram::dg_sender_t senderId = senderId_;

  // Split frame into datagrams.
  UDP::VideoFrameDatagram **datagrams = 0;
  UDP::VideoFrameDatagram::dg_data_count_t datagramsLength;
  if (UDP::VideoFrameDatagram::split((UDP::dg_byte_t*)frame_.data(), frame_.size(), frameId, senderId, &datagrams, datagramsLength) != 0) {
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
  UDP::VideoFrameDatagram::freeData(datagrams, datagramsLength);
}

void MediaSocket::timerEvent(QTimerEvent *ev)
{
  if (ev->timerId() == _authenticationTimerId) {
    sendAuthTokenDatagram(_token);
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

    //qDebug() << QString("Incoming datagram (size=%1)").arg(data.size());

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

      // Video data.
      case UDP::VideoFrameDatagram::TYPE: {
        UDP::VideoFrameDatagram dgvideo;
        in >> dgvideo.flags;
        in >> dgvideo.sender;
        in >> dgvideo.frameId;
        in >> dgvideo.index;
        in >> dgvideo.count;
        in >> dgvideo.size;
        if (dgvideo.size > 0) {
          dgvideo.data = new UDP::dg_byte_t[dgvideo.size];
          in.readRawData((char*)dgvideo.data, dgvideo.size);
        }
        break;
      }

    }
  }
}

///////////////////////////////////////////////////////////////////////

VideoEncodingThread::VideoEncodingThread(QObject *parent) :
  QThread(parent),
  _stopFlag(0)
{
}

VideoEncodingThread::~VideoEncodingThread()
{
  stop();
}

void VideoEncodingThread::stop()
{
  _stopFlag = 1;
  _queueCond.wakeAll();
}

void VideoEncodingThread::enqueue(const QImage &image)
{
  QMutexLocker l(&_m);
  _queue.enqueue(image);
  while (_queue.size() > 5) {
    _queue.dequeue();
  }
  l.unlock();
  _queueCond.wakeAll();
}

void VideoEncodingThread::run()
{
  while (_stopFlag == 0) {
    QMutexLocker l(&_m);
    if (_queue.isEmpty()) {
      _queueCond.wait(&_m);
    }
    auto image = _queue.dequeue();
    l.unlock();

    if (image.isNull()) {
      qDebug() << QString("Invalid image");
      continue;
    }

    // TODO Convert to YuvFrame.
    // TODO Encode via VPX.
    // TODO emit signal

    // DEV
    if (true) {
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out << image;
      emit newEncodedFrame(data);
    }

  }
}