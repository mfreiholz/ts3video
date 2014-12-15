#include "mytestobject.h"
#include "qcorserver.h"
#include "qcorconnection.h"
#include "qcorframe.h"


MyTestObject::MyTestObject(QObject *parent) :
  QObject(parent),
  _server(new QCorServer(this)),
  _clientConn(0)
{
  _server->listen(QHostAddress::Any, 5005);
  connect(_server, SIGNAL(newConnection(QCorConnection *)), SLOT(onNewConnection(QCorConnection *)));
}

void MyTestObject::onNewConnection(QCorConnection *conn)
{
  qDebug() << "new connection";
}

void MyTestObject::clientConnect()
{
  _clientConn = new QCorConnection(this);
  _clientConn->connectTo(QHostAddress::LocalHost, 5005);
}

void MyTestObject::clientSendFrame()
{
  
}