#include <QDateTime>
#include <QTimer>
#include "mytestobject.h"
#include "qcorserver.h"
#include "qcorconnection.h"
#include "qcorrequest.h"


MyTestObject::MyTestObject(QObject *parent) :
  QObject(parent),
  _server(0),
  _receivedFrames(0),
  _receivedFrameBytes(0)
{
}

void MyTestObject::startServer()
{
  _server = new QCorServer(this);
  _server->listen(QHostAddress::Any, 5005);
  connect(_server, SIGNAL(newConnection(QCorConnection *)), SLOT(onNewConnection(QCorConnection *)));

  QTimer *t = new QTimer(this);
  connect(t, SIGNAL(timeout()), SLOT(printServerStatistics()));
  t->start(1000);
}

void MyTestObject::onNewConnection(QCorConnection *conn)
{
  _serverConnections.append(conn);
  connect(conn, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onConnectionStateChanged(QAbstractSocket::SocketState)));
  connect(conn, SIGNAL(newIncomingRequest(QCorFrameRefPtr)), SLOT(onNewFrame(QCorFrameRefPtr)));
}

void MyTestObject::onConnectionStateChanged(QAbstractSocket::SocketState state)
{
  QCorConnection *conn = static_cast<QCorConnection*>(sender());
  if (state == QAbstractSocket::UnconnectedState) {
    _serverConnections.removeAll(conn);
  }
}

void MyTestObject::printServerStatistics()
{
  qDebug() << QString("[%1] connections: %2; rec-frames: %3; rec-frame-bytes: %4")
    .arg(QDateTime::currentDateTime().toString("m"))
    .arg(_serverConnections.count())
    .arg(_receivedFrames)
    .arg(_receivedFrameBytes);
}

void MyTestObject::onNewFrame(QCorFrameRefPtr frame)
{
  QCorConnection *conn = qobject_cast<QCorConnection*>(sender());

  _receivedFrames++;
  _receivedFrameBytes += frame->data().size();

  // Respond with echo.
  QCorFrame res;
  res.setCorrelationId(frame->correlationId());
  res.setData(frame->data());
  conn->sendResponse(res);
}

// Client based from here...

void MyTestObject::clientConnect(int testRequestInterval)
{
  if (_clientConnections.count() >= 200) {
    return;
  }

  QCorConnection *conn = new QCorConnection(this);
  conn->connectTo(QHostAddress("85.214.204.236"), 5005);
  _clientConnections.append(conn);

  QTimer *timer = new QTimer(this);
  QObject::connect(timer, SIGNAL(timeout()), conn, SLOT(sendTestRequest()));
  timer->setSingleShot(false);
  timer->start(testRequestInterval);
}