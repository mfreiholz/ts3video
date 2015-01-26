#include "mediasockethandler.h"

#include <QDebug>

#include "ts3videoserver.h"

///////////////////////////////////////////////////////////////////////

MediaSocketHandler::MediaSocketHandler(quint16 port, QObject *parent) :
  QObject(parent),
  _socket(this)
{
  if (!_socket.bind(QHostAddress::Any, port, QAbstractSocket::DontShareAddress)) {
    qDebug() << QString("Can not bind media UDP socket on 6001");
  }
  connect(&_socket, &QUdpSocket::readyRead, this, &MediaSocketHandler::onReadyRead);
}

MediaSocketHandler::~MediaSocketHandler()
{
  _socket.close();
}

void MediaSocketHandler::setRecipients(const MediaRecipients &rec)
{
  _recipients = rec;
}

void MediaSocketHandler::onReadyRead()
{
  while (_socket.hasPendingDatagrams()) {
    // Read datagram.
    QByteArray data;
    QHostAddress senderAddress;
    quint16 senderPort;
    data.resize(_socket.pendingDatagramSize());
    _socket.readDatagram(data.data(), data.size(), &senderAddress, &senderPort);

    qDebug() << QString("Incoming datagram: %1").arg(QString(data));

    // TODO Handle datagram by type.
    // TODO Handle authentication.
    if (false) {
      auto token = QString("foobar");
      emit tokenAuthentication(token, senderAddress, senderPort);
    }
    // Handle video datagram.
    else if (false) {
      auto senderId = MediaSenderEntity::createID(senderAddress, senderPort);
      const auto &senderEntity = _recipients.id2sender[senderId];
      for (auto i = 0; i < senderEntity.receivers.size(); ++i) {
        const auto &receiverEntity = senderEntity.receivers[i];
        _socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
      }
    }
  }
}