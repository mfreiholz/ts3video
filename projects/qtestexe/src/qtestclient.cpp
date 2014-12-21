#include <QTimer>
#include "qcorrequest.h"
#include "qcorreply.h"
#include "qtestclient.h"

QTestClient::QTestClient(QObject *parent) : QObject(parent)
{
  _connection = 0;
}

QTestClient::~QTestClient()
{

}

void QTestClient::connectToHost(const QHostAddress &address, quint16 port)
{
  _connection = new QCorConnection(this);
  connect(_connection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onStateChanged(QAbstractSocket::SocketState)));
  _connection->connectTo(address, port);
}

void QTestClient::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      sendTestFrame();
      break;
  }
}

void QTestClient::sendTestFrame()
{
  QCorFrame frame;
  frame.setData(QByteArray("HeY Ho Buddy!"));
  
  QCorReply *res = _connection->sendRequest(frame);
  connect(res, SIGNAL(finished()), SLOT(onResponseFinished()));
}

void QTestClient::onResponseFinished()
{
  QCorReply *res = qobject_cast<QCorReply*>(sender());
  res->deleteLater();
  
  qDebug() << QString("response (%1 ms): %2")
    .arg(res->elapsedMillis())
    .arg(QString::fromUtf8(res->frame()->data()));

  QTimer::singleShot(250, this, SLOT(sendTestFrame()));
}