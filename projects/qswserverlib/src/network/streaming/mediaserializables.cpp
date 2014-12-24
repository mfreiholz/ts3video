#include "mediaserializables.h"

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////

QDataStream& operator<<(QDataStream &stream, const UdpDataSender &sender)
{
  stream << sender.client_id;
  stream << (int) sender.receivers.size();
  foreach (const UdpDataReceiver &receiver, sender.receivers) {
    stream << receiver;
  }
  return stream;
}

QDataStream& operator>>(QDataStream &stream, UdpDataSender &sender)
{
  stream >> sender.client_id;
  int receiversCount = 0;
  stream >> receiversCount;
  for (int i = 0; i < receiversCount; ++i) {
    UdpDataReceiver receiver;
    stream >> receiver;
    sender.receivers.append(receiver);
  }
  return stream;
}

QDataStream& operator<<(QDataStream &stream, const UdpDataReceiver &receiver)
{
  stream << receiver.client_id;
  stream << receiver.identifier;
  stream << receiver.address;
  stream << receiver.port;
  return stream;
}

QDataStream& operator>>(QDataStream &stream, UdpDataReceiver &receiver)
{
  stream >> receiver.client_id;
  stream >> receiver.identifier;
  stream >> receiver.address;
  stream >> receiver.port;
  return stream;
}