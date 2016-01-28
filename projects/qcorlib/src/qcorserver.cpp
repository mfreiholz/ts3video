#include <QTcpSocket>
#include "qcorlib/qcorserver.h"
#include "qcorserver_p.h"
#include "qcorlib/qcorconnection.h"

QCorServer::QCorServer(QObject *parent) :
  QTcpServer(parent),
  d(new QCorServerPrivate(this))
{
}

QCorServer::~QCorServer()
{
  delete d;
}

void QCorServer::incomingConnection(qintptr socketDescriptor)
{
  QCorConnection *conn = new QCorConnection(this);
  conn->connectWith(socketDescriptor);
  emit newConnection(conn);
}

///////////////////////////////////////////////////////////////////////
// Private Object
///////////////////////////////////////////////////////////////////////

QCorServerPrivate::QCorServerPrivate(QCorServer *owner) :
  QObject(owner),
  owner(owner)
{
}