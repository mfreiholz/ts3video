#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

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

#include "medprotocol.h"

#include "qcorconnection.h"
#include "qcorreply.h"

#include "ts3video.h"
#include "vp8encoder.h"
#include "vp8decoder.h"
#include "timeutil.h"

HUMBLE_LOGGER(HL, "client.ts3videoclient");

///////////////////////////////////////////////////////////////////////

TS3VideoClient::TS3VideoClient(QObject *parent) :
  QObject(parent),
  d(new TS3VideoClientPrivate(this))
{
  qRegisterMetaType<YuvFrameRefPtr>("YuvFrameRefPtr");
  qRegisterMetaType<NetworkUsageEntity>("NetworkUsageEntity");

  d->corSocket = new QCorConnection(this);
  connect(d->corSocket, &QCorConnection::stateChanged, this, &TS3VideoClient::onStateChanged);
  connect(d->corSocket, &QCorConnection::error, this, &TS3VideoClient::error);
  connect(d->corSocket, &QCorConnection::newIncomingRequest, this, &TS3VideoClient::onNewIncomingRequest);

  d->heartbeatTimer.setInterval(10000);
  QObject::connect(&d->heartbeatTimer, &QTimer::timeout, this, &TS3VideoClient::sendHeartbeat);
}

TS3VideoClient::~TS3VideoClient()
{
  delete d->corSocket;
  delete d->mediaSocket;
}

void TS3VideoClient::setMediaEnabled(bool yesno)
{
  d->useMediaSocket = yesno;
}

const QAbstractSocket* TS3VideoClient::socket() const
{
  return d->corSocket->socket();
}

const ClientEntity& TS3VideoClient::clientEntity() const
{
  return d->clientEntity;
}

bool TS3VideoClient::isReadyForStreaming() const
{
  if (!d->mediaSocket || d->mediaSocket->state() != QAbstractSocket::ConnectedState) {
    return false;
  }
  if (!d->mediaSocket->isAuthenticated()) {
    return false;
  }
  return true;
}

void TS3VideoClient::connectToHost(const QHostAddress &address, qint16 port)
{
  Q_ASSERT(!address.isNull());
  Q_ASSERT(port > 0);
  d->corSocket->connectTo(address, port);
}

QCorReply* TS3VideoClient::auth(const QString &name, const QString &password)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);

  QJsonObject params;
  params["version"] = IFVS_SOFTWARE_VERSION;
  params["username"] = name;
  params["password"] = password;
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
      QObject::connect(d->mediaSocket, &MediaSocket::newVideoFrame, this, &TS3VideoClient::newVideoFrame);
      QObject::connect(d->mediaSocket, &MediaSocket::networkUsageUpdated, this, &TS3VideoClient::networkUsageUpdated);
    }
  });

  return reply;
}

QCorReply* TS3VideoClient::joinChannel(int id)
{
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);
  QJsonObject params;
  params["channelid"] = id;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->corSocket->sendRequest(req);
}

void TS3VideoClient::sendVideoFrame(const QImage &image)
{
  Q_ASSERT(d->mediaSocket);
  if (d->mediaSocket) {
    d->mediaSocket->sendVideoFrame(image, d->clientEntity.id);
  }
}

void TS3VideoClient::sendHeartbeat()
{
  Q_ASSERT(d->corSocket->socket()->state() == QAbstractSocket::ConnectedState);
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("heartbeat", QJsonObject()));
  auto reply = d->corSocket->sendRequest(req);
  QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
}

void TS3VideoClient::onStateChanged(QAbstractSocket::SocketState state)
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

void TS3VideoClient::onNewIncomingRequest(QCorFrameRefPtr frame)
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

///////////////////////////////////////////////////////////////////////

MediaSocket::MediaSocket(const QString &token, QObject *parent) :
  QUdpSocket(parent),
  _authenticated(false),
  _token(token),
  _authenticationTimerId(-1),
  _videoEncodingThread(new VideoEncodingThread(this)),
  _videoDecodingThread(new VideoDecodingThread(this)),
  _lastFrameRequestTimestamp(0),
  _networkUsage(),
  _networkUsageHelper(_networkUsage)
{
  connect(this, &MediaSocket::stateChanged, this, &MediaSocket::onSocketStateChanged);
  connect(this, &MediaSocket::readyRead, this, &MediaSocket::onReadyRead);

  static quint64 __frameId = 1;
  _videoEncodingThread->start();
  connect(_videoEncodingThread, &VideoEncodingThread::encoded, [this] (const QByteArray &frame, int senderId) {
    sendVideoFrame(frame, __frameId++, senderId);
  });
  
  _videoDecodingThread->start();
  connect(_videoDecodingThread, &VideoDecodingThread::decoded, this, &MediaSocket::newVideoFrame);

  // Network usage calculation.
  auto bandwidthTimer = new QTimer(this);
  bandwidthTimer->setInterval(1500);
  bandwidthTimer->start();
  QObject::connect(bandwidthTimer, &QTimer::timeout, [this] () {
    _networkUsageHelper.recalculate();
    emit networkUsageUpdated(_networkUsage);
  });
}

MediaSocket::~MediaSocket()
{
  if (_authenticationTimerId != -1) {
    killTimer(_authenticationTimerId);
  }

  while (!_videoFrameDatagramDecoders.isEmpty()) {
    auto obj = _videoFrameDatagramDecoders.take(_videoFrameDatagramDecoders.begin().key());
    delete obj;
  }

  if (_videoEncodingThread) {
    _videoEncodingThread->stop();
    _videoEncodingThread->wait();
    delete _videoEncodingThread;
  }

  if (_videoDecodingThread) {
    _videoDecodingThread->stop();
    _videoDecodingThread->wait();
    delete _videoDecodingThread;
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

void MediaSocket::sendVideoFrame(const QImage &image, int senderId)
{
  if (!_videoEncodingThread || !_videoEncodingThread->isRunning()) {
    HL_WARN(HL, QString("Can not send video. Encoding thread not yet running.").toStdString());
    return;
  }
  _videoEncodingThread->enqueue(image, senderId);
}

void MediaSocket::sendAuthTokenDatagram(const QString &token)
{
  Q_ASSERT(!token.isEmpty());
  HL_DEBUG(HL, QString("Send media auth token (token=%1; address=%2; port=%3)").arg(token).arg(peerAddress().toString()).arg(peerPort()).toStdString());

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
  auto written = writeDatagram(datagram, peerAddress(), peerPort());
  if (written > 0)
    _networkUsage.bytesWritten += written;
}

void MediaSocket::sendVideoFrame(const QByteArray &frame_, quint64 frameId_, quint32 senderId_)
{
  Q_ASSERT(frame_.isEmpty() == false);
  Q_ASSERT(frameId_ != 0);

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
    auto written = writeDatagram(datagram, peerAddress(), peerPort());
    if (written > 0)
      _networkUsage.bytesWritten += written;
  }
  UDP::VideoFrameDatagram::freeData(datagrams, datagramsLength);
}

void MediaSocket::sendVideoFrameRecoveryDatagram(quint64 frameId_, quint32 fromSenderId_)
{
  Q_ASSERT(frameId_ != 0);
  Q_ASSERT(fromSenderId_ != 0);

  UDP::VideoFrameRecoveryDatagram dgrec;
  dgrec.sender = fromSenderId_;
  dgrec.frameId = frameId_;
  dgrec.index = 0;

  QByteArray datagram;
  QDataStream out(&datagram, QIODevice::WriteOnly);
  out.setByteOrder(QDataStream::BigEndian);
  out << dgrec.magic;
  out << dgrec.type;
  out << dgrec.sender;
  out << dgrec.frameId;
  out << dgrec.index;
  auto written = writeDatagram(datagram, peerAddress(), peerPort());
  if (written > 0)
    _networkUsage.bytesWritten += written;
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
    auto read = readDatagram(data.data(), data.size(), &senderAddress, &senderPort);
    if (read > 0)
      _networkUsage.bytesRead += read;

    QDataStream in(data);
    in.setByteOrder(QDataStream::BigEndian);

    // Check magic.
    UDP::Datagram dg;
    in >> dg.magic;
    if (dg.magic != UDP::Datagram::MAGIC) {
      HL_WARN(HL, QString("Received invalid datagram (size=%1; data=%2)").arg(data.size()).arg(QString(data)).toStdString());
      continue;
    }

    // Handle by type.
    in >> dg.type;
    switch (dg.type) {

      // Video data.
      case UDP::VideoFrameDatagram::TYPE: {
        // Parse datagram.
        auto dgvideo = new UDP::VideoFrameDatagram();
        in >> dgvideo->flags;
        in >> dgvideo->sender;
        in >> dgvideo->frameId;
        in >> dgvideo->index;
        in >> dgvideo->count;
        in >> dgvideo->size;
        if (dgvideo->size > 0) {
          dgvideo->data = new UDP::dg_byte_t[dgvideo->size];
          in.readRawData((char*)dgvideo->data, dgvideo->size);
        }
        if (dgvideo->size == 0) {
          delete dgvideo;
          continue;
        }

        auto senderId = dgvideo->sender;
        auto frameId =dgvideo->frameId;

        // UDP Decode.
        auto decoder = _videoFrameDatagramDecoders.value(dgvideo->sender);
        if (!decoder) {
          decoder = new VideoFrameUdpDecoder();
          _videoFrameDatagramDecoders.insert(dgvideo->sender, decoder);
        }
        decoder->add(dgvideo);

        // Check for new decoded frame.
        auto frame = decoder->next();
        auto waitForType = decoder->getWaitsForType();
        if (frame) {
          _videoDecodingThread->enqueue(frame, senderId);
        }

        // Handle the case, that the UDP decoder requires some special data.
        if (waitForType != VP8Frame::NORMAL) {
          // Request recovery frame (for now only key-frames).
          auto now = get_local_timestamp();
          if (get_local_timestamp_diff(_lastFrameRequestTimestamp, now) > 1000) {
            _lastFrameRequestTimestamp = now;
            sendVideoFrameRecoveryDatagram(frameId, senderId);
          }
        }
        break;
      }

      // Video recovery.
      case UDP::VideoFrameRecoveryDatagram::TYPE: {
        UDP::VideoFrameRecoveryDatagram dgrec;
        in >> dgrec.sender;
        in >> dgrec.frameId;
        in >> dgrec.index;
        _videoEncodingThread->enqueueRecovery();
        break;
      }

    }
  }
}

///////////////////////////////////////////////////////////////////////

VideoEncodingThread::VideoEncodingThread(QObject *parent) :
  QThread(parent),
  _stopFlag(0),
  _recoveryFlag(VP8Frame::NORMAL)
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

void VideoEncodingThread::enqueue(const QImage &image, int senderId)
{
  QMutexLocker l(&_m);
  _queue.enqueue(qMakePair(image, senderId));
  while (_queue.size() > 5) {
    auto item = _queue.dequeue();
  }
  _queueCond.wakeAll();
}

void VideoEncodingThread::enqueueRecovery()
{
  _recoveryFlag = VP8Frame::KEY;
  _queueCond.wakeAll();
}

void VideoEncodingThread::run()
{
  const int fps = 20;
  const int fpsTimeMs = 1000 / fps;
  const int bitRate = 100;
  const QSize dim(640, 480);

  QHash<int, VP8Encoder*> encoders;
  QTime fpsTimer;
  fpsTimer.start();

  _stopFlag = 0;
  while (_stopFlag == 0) {
    QMutexLocker l(&_m);
    if (_queue.isEmpty()) {
      _queueCond.wait(&_m);
      continue;
    }
    auto item = _queue.dequeue();
    l.unlock();

    if (item.first.isNull()) {
      continue;
    }

    if (fps > 0 && fpsTimer.elapsed() < fpsTimeMs) {
      continue;
    }
    fpsTimer.restart();

    if (true) {
      // Convert to YuvFrame.
      auto yuv = YuvFrame::fromQImage(item.first);
      // Encode via VPX.
      auto encoder = encoders.value(item.second);
      if (!encoder) {
        HL_DEBUG(HL, QString("Create new VP8 video encoder (id=%1)").arg(item.second).toStdString());
        encoder = new VP8Encoder();
        if (!encoder->initialize(dim.width(), dim.height(), bitRate, fps)) { ///< TODO Find a way to pass this parameters from outside (videoBegin(...), sendVideo(...), videoEnd(...)).
          HL_ERROR(HL, QString("Can not initialize VP8 video encoder").toStdString());
          _stopFlag = 1;
          continue;
        }
        encoders.insert(item.second, encoder);
      }
      if (_recoveryFlag != VP8Frame::NORMAL) {
        encoder->setRequestRecoveryFlag(VP8Frame::KEY);
        _recoveryFlag = VP8Frame::NORMAL;
      }
      auto vp8 = encoder->encode(*yuv);
      delete yuv;
      // Serialize VP8Frame.
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out << *vp8;
      delete vp8;
      emit encoded(data, item.second);
    }
    
    // DEV Provides plain QImage.
    //if (true) {
    //  static quint64 __frameTime = 1;
    //  VP8Frame vpframe;
    //  vpframe.time = __frameTime++;
    //  vpframe.type = VP8Frame::KEY;
    //  QDataStream out(&vpframe.data, QIODevice::WriteOnly);
    //  out << item.first;

    //  QByteArray data;
    //  QDataStream out2(&data, QIODevice::WriteOnly);
    //  out2 << vpframe;

    //  emit encoded(data, item.second);
    //}

  }
}

///////////////////////////////////////////////////////////////////////

VideoDecodingThread::VideoDecodingThread(QObject *parent) :
  QThread(parent)
{

}

VideoDecodingThread::~VideoDecodingThread()
{
  stop();
}

void VideoDecodingThread::stop()
{
  _stopFlag = 1;
  _queueCond.wakeAll();
}

void VideoDecodingThread::enqueue(VP8Frame *frame, int senderId)
{
  QMutexLocker l(&_m);
  _queue.enqueue(qMakePair(frame, senderId));
  //while (_queue.size() > 5) {
  //  auto item = _queue.dequeue();
  //  delete item.first;
  //}
  _queueCond.wakeAll();
}

void VideoDecodingThread::run()
{
  QHash<int, VP8Decoder*> decoders;

  _stopFlag = 0;
  while (_stopFlag == 0) {
    QMutexLocker l(&_m);
    if (_queue.isEmpty()) {
      _queueCond.wait(&_m);
      continue;
    }
    auto item = _queue.dequeue();
    l.unlock();

    if (!item.first || item.second == 0) {
      continue;
    }

    if (true) {
      // Decode VPX frame to YuvFrame.
      auto decoder = decoders.value(item.second);
      if (!decoder) {
        decoder = new VP8Decoder();
        decoder->initialize();
        decoders.insert(item.second, decoder);
      }
      auto yuv = YuvFrameRefPtr(decoder->decodeFrameRaw(item.first->data));
      //auto image = yuv->toQImage(); ///< TODO This call is VERY instense! We may want to work with YuvFrame's directly.
      emit decoded(yuv, item.second);
    }

    // DEV
    //if (true) {
    //  QImage image;
    //  QDataStream in(item.first->data);
    //  in >> image;
    //  delete item.first;
    //  emit decoded(image, item.second);
    //}

  }
}