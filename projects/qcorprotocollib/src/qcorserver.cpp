#include <QTcpSocket>
#include "qcorserver.h"
#include "qcorconnection.h"

QCorServer::QCorServer(QObject *parent) :
  QTcpServer(parent)
{
}

void QCorServer::incomingConnection(qintptr socketDescriptor)
{
  QCorConnection *conn = new QCorConnection(this);
  conn->connectWith(socketDescriptor);
  emit newConnection(conn);
}