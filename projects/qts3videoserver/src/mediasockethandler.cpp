#include "mediasockethandler.h"

#include <QDebug>
#include <QString>
#include <QDataStream>

#include "medprotocol.h"

#include "ts3videoserver.h"

///////////////////////////////////////////////////////////////////////

MediaSocketHandler::MediaSocketHandler(quint16 port, QObject *parent) :
  QObject(parent),
  _socket(this)
{
  if (!_socket.bind(QHostAddress::Any, port, QAbstractSocket::DontShareAddress)) {
    qDebug() << QString("Can not bind media UDP socket on %1").arg(port);
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

      // Authentication
      case UDP::AuthDatagram::TYPE: {
        UDP::AuthDatagram dgauth;
        in >> dgauth.size;
        dgauth.data = new UDP::dg_byte_t[dgauth.size];
        auto read = in.readRawData((char*)dgauth.data, dgauth.size);
        if (read != dgauth.size) {
          // Error.
          continue;
        }
        auto token = QString::fromUtf8((char*)dgauth.data, dgauth.size);
        emit tokenAuthentication(token, senderAddress, senderPort);
        break;
      }

      // Video data.
      case UDP::VideoFrameDatagram::TYPE: {
        auto senderId = MediaSenderEntity::createID(senderAddress, senderPort);
        const auto &senderEntity = _recipients.id2sender[senderId];
        for (auto i = 0; i < senderEntity.receivers.size(); ++i) {
          const auto &receiverEntity = senderEntity.receivers[i];
          _socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
        }
        break;
      }
    }

  } // while (datagrams)
}