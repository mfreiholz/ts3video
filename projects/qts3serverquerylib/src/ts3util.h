#ifndef TS3UTIL_H
#define TS3UTIL_H

class QByteArray;
class QTcpSocket;

namespace TS3Util
{

bool test1();

QByteArray socketReadLineSync(QTcpSocket& s);

}

#endif