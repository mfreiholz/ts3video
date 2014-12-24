#ifndef MEDIASERIALIZABLES_HEADER
#define MEDIASERIALIZABLES_HEADER

#include "QVector"
#include "QHash"
#include "QString"
#include "QHostAddress"
#include "shared/network/protocol.h"
class UdpDataSender;
class UdpDataReceiver;


class UdpDataSender
{
public:
  Protocol::client_id_t client_id;
  QVector<UdpDataReceiver> receivers;
};


class UdpDataReceiver
{
public:
  Protocol::client_id_t client_id;
  QString identifier;
  QHostAddress address;
  quint16 port;
};


// QHash which associates a sender to all receivers.
// Key = String concatination of "<ip-address>:<port>".
typedef QHash<QString, UdpDataSender> UdpDataReceiverMap;
QString CreateUdpDataReceiverMapKey(const QHostAddress &address, quint16 port);


// Serialization functions.
QDataStream& operator<<(QDataStream &stream, const UdpDataSender &sender);
QDataStream& operator>>(QDataStream &stream, UdpDataSender &sender);

QDataStream& operator<<(QDataStream &stream, const UdpDataReceiver &receiver);
QDataStream& operator>>(QDataStream &stream, UdpDataReceiver &receiver);


Q_DECLARE_METATYPE(UdpDataSender)
Q_DECLARE_METATYPE(UdpDataReceiver)
Q_DECLARE_METATYPE(UdpDataReceiverMap)
#endif