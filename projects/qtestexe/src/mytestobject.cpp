#include <QDateTime>
#include <QTimer>
#include "mytestobject.h"
#include "qcorserver.h"
#include "qcorconnection.h"
#include "qcorframe.h"


MyTestObject::MyTestObject(QObject *parent) :
  QObject(parent),
  _server(new QCorServer(this))
{
  _server->listen(QHostAddress::Any, 5005);
  connect(_server, SIGNAL(newConnection(QCorConnection *)), SLOT(onNewConnection(QCorConnection *)));

  QTimer *t = new QTimer(this);
  connect(t, SIGNAL(timeout()), SLOT(printStatistics()));
  t->start(1000);
}

void MyTestObject::onNewConnection(QCorConnection *conn)
{
  //qDebug() << QString("%1 new connection").arg(QDateTime::currentDateTime().toString());
  _serverConnections.append(conn);
  connect(conn, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onConnectionStateChanged(QAbstractSocket::SocketState)));
  connect(conn, SIGNAL(newFrame(QCorFrame *)), SLOT(onNewFrame(QCorFrame *)));
}

void MyTestObject::onConnectionStateChanged(QAbstractSocket::SocketState state)
{
  qDebug() << QString("%1 connection state changed").arg(QDateTime::currentDateTime().toString());
  QCorConnection *conn = static_cast<QCorConnection*>(sender());
  if (state == QAbstractSocket::UnconnectedState) {
    _serverConnections.removeAll(conn);
  }
}

void MyTestObject::onNewFrame(QCorFrame *frame)
{
  //qDebug() << QString("%1 new frame").arg(QDateTime::currentDateTime().toString());
  QObject::connect(frame, SIGNAL(end()), frame, SLOT(deleteLater()));
}

void MyTestObject::clientConnect()
{
  QCorConnection *conn = new QCorConnection(this);
  conn->connectTo(QHostAddress::LocalHost, 5005);

  QTimer *timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()), conn, SLOT(sendTestRequest()));
  timer->start(100);
}

void MyTestObject::printStatistics()
{
  qDebug() << QString("[%1] connections: %2").arg(QDateTime::currentDateTime().toString()).arg(_serverConnections.count());
}