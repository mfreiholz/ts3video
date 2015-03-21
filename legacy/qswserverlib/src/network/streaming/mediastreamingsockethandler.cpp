#include "mediastreamingsockethandler_p.h"
#include "QUdpSocket"
#include "humblelogging/api.h"
#include "shared/network/udp_streaming_protocol.h"

HUMBLE_LOGGER(HL, "server.streaming");

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////

QString getReceiverKey(const QHostAddress &address, quint16 port)
{
  return address.toString() + QString(":") + QString::number(port);
}

///////////////////////////////////////////////////////////////////////////////
// MediaStreamingSocketHandler
///////////////////////////////////////////////////////////////////////////////

MediaStreamingSocketHandler::MediaStreamingSocketHandler(QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  qRegisterMetaType<UdpDataReceiverMap>("UdpDataReceiverMap");
}

MediaStreamingSocketHandler::~MediaStreamingSocketHandler()
{
  delete d;
}

bool MediaStreamingSocketHandler::listen(const QHostAddress &address, quint16 port)
{
  return d->_socket->bind(address, port);
}

void MediaStreamingSocketHandler::close()
{
  if (d->_socket) {
    d->_socket->close();
  }
}

void MediaStreamingSocketHandler::setReceiverMap(const UdpDataReceiverMap &map)
{
  d->_receiverMap = map;
}

void MediaStreamingSocketHandler::clearReceivers()
{
  d->_receiverMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MediaStreamingSocketHandler::Private::Private(MediaStreamingSocketHandler *owner) :
  QObject(owner),
  _owner(owner),
  _socket(new QUdpSocket(this)),
  _totalBytesRead(0),
  _totalBytesWritten(0)
{
  connect(_socket, SIGNAL(readyRead()), SLOT(onReadyReadDatagrams()));
  connect(_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onStateChanged(QAbstractSocket::SocketState)));
}

MediaStreamingSocketHandler::Private::~Private()
{
  if (_socket->state() == QAbstractSocket::BoundState) {
    _socket->close();
  }
  delete _socket;
}

void MediaStreamingSocketHandler::Private::onStateChanged(QAbstractSocket::SocketState state)
{
  if (state == QAbstractSocket::UnconnectedState) {
    emit _owner->done();
  }
}

void MediaStreamingSocketHandler::Private::onReadyReadDatagrams()
{
  while (_socket->hasPendingDatagrams()) {
    QHostAddress sender;
    quint16 senderPort;
    QByteArray data;
    data.resize(_socket->pendingDatagramSize());

    const qint64 bytesRead = _socket->readDatagram(data.data(), data.size(), &sender, &senderPort);
    processDatagram(&data, sender, senderPort);

    _totalBytesRead += bytesRead;
  }
}

bool MediaStreamingSocketHandler::Private::processDatagram(QByteArray *data, const QHostAddress &sender, quint16 sender_port)
{
  QDataStream in(*data);
  in.setVersion(UdpProtocol::QDS_VERSION);

  UdpProtocol::UdpDatagramHeader header;
  in >> header;

  switch (header.type) {
    case UdpProtocol::DG_TYPE_TOKEN:
      return processTokenDatagram(in, data, sender, sender_port);
    case UdpProtocol::DG_TYPE_VIDEO_FRAME_V1:
      return processVideoFrameDatagramV1(in, data, sender, sender_port);
    case UdpProtocol::DG_TYPE_VIDEO_FRAME_RECOVERY_V1:
      return processVideoFrameRecoveryDatagramV1(in, data, sender, sender_port);
    case UdpProtocol::DG_TYPE_AUDIO_FRAME_V1:
      return processAudioFrameDatagramV1(in, data, sender, sender_port);
    default:
      HL_ERROR(HL, QString("Invalid datagram (sender=%1; port=%2)").arg(sender.toString()).arg(sender_port).toStdString());
      break;
  }
  return false;
}

bool MediaStreamingSocketHandler::Private::processTokenDatagram(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port)
{
  UdpProtocol::TokenUdpDatagram datagram;
  in >> datagram;
  emit _owner->tokenAuthorization(datagram.token, sender, sender_port);
  return true;
}

bool MediaStreamingSocketHandler::Private::processVideoFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender_address, quint16 sender_port)
{
  // Find detailed sender information (+receiver list).
  const QString sender_identifier = getReceiverKey(sender_address, sender_port);
  const UdpDataSender &sender_data = _receiverMap.value(sender_identifier);
  if (sender_data.receivers.isEmpty()) {
    return true;
  }

  // Rewrite senders client-id in package.
  // Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(UdpProtocol::QDS_VERSION);
  ds << sender_data.client_id;
  data->replace(sizeof(UdpProtocol::UdpDatagramHeader), sizeof(Protocol::client_id_t), ba);

  // Send data to sibling clients.
  std::for_each(sender_data.receivers.begin(), sender_data.receivers.end(), [&] (const UdpDataReceiver &receiver) {
    const qint64 bytesWritten = _socket->writeDatagram(*data, receiver.address, receiver.port);
    if (bytesWritten != -1)
      _totalBytesWritten += bytesWritten;
  });
  return true;
}

bool MediaStreamingSocketHandler::Private::processVideoFrameRecoveryDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender_address, quint16 sender_port)
{
  const QString sender_identifier = getReceiverKey(sender_address, sender_port);
  const UdpDataSender &sender_data = _receiverMap.value(sender_identifier);
  if (sender_data.receivers.isEmpty()) {
    return true;
  }

  // Rewrite senders client-id in package.
  // Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(UdpProtocol::QDS_VERSION);
  ds << sender_data.client_id;

  data->replace(sizeof(UdpProtocol::UdpDatagramHeader), sizeof(Protocol::client_id_t), ba);

  // Only send to the client, which has to resend the frame.
  Protocol::client_id_t sender_id, receiver_id;
  in >> sender_id >> receiver_id;

  for (int i = 0; i < sender_data.receivers.size(); ++i) {
    const UdpDataReceiver &receiver = sender_data.receivers.at(i);
    if (receiver.client_id != receiver_id)
      continue;
    const qint64 bytesWritten = _socket->writeDatagram(*data, receiver.address, receiver.port);
    if (bytesWritten != -1)
      _totalBytesWritten += bytesWritten;
    break;
  }
  return true;
}

bool MediaStreamingSocketHandler::Private::processAudioFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &senderAddress, quint16 senderPort)
{
  // Find detailed sender information (+receiver list).
  const QString sender_identifier = getReceiverKey(senderAddress, senderPort);
  const UdpDataSender &senderData = _receiverMap.value(sender_identifier);
  if (senderData.receivers.isEmpty()) {
    return true;
  }

  // Rewrite senders client-id in package.
  // Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(UdpProtocol::QDS_VERSION);
  ds << senderData.client_id;
  data->replace(sizeof(UdpProtocol::UdpDatagramHeader), sizeof(Protocol::client_id_t), ba);

  // Send data to sibling clients.
  std::for_each(senderData.receivers.begin(), senderData.receivers.end(), [&] (const UdpDataReceiver &receiver) {
    const qint64 bytesWritten = _socket->writeDatagram(*data, receiver.address, receiver.port);
    if (bytesWritten != -1)
      _totalBytesWritten += bytesWritten;
  });
  return true;
}