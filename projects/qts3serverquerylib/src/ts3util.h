#ifndef TS3UTIL_H
#define TS3UTIL_H

#include <QtGlobal>

class QByteArray;
class QString;
class QHostAddress;
class QTcpSocket;

namespace TS3Util
{

/*!
  Connects to server-query-console and checks whether the client is on the server.

    login serveradmin TiHxQDHt
    use 9987
    clientlist -ip
    quit
*/
bool isClientConnected(const QHostAddress& address, quint16 port, const QString& loginName, const QString& loginPassword, quint16 vsPort, const QString& clientIp);

/*!
*/
QByteArray socketReadLineSync(QTcpSocket& s);

}
#endif