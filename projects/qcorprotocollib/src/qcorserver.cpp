#include <QTcpSocket>
#include "qcorserver.h"
#include "qcorconnection.h"

QCorServer::QCorServer(QObject *parent) :
  QTcpServer(parent)
{

}

void QCorServer::incomingConnection(qintptr socketDescriptor)
{
  QTcpSocket *socket = new QTcpSocket(this);
  socket->setSocketDescriptor(socketDescriptor);
  socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

  QCorConnection *conn = new QCorConnection(socket, this);
  connect(conn, SIGNAL(newFrame(QCorFrame*)), SIGNAL(newFrame(QCorFrame*)));
  emit newConnection(conn);
}
