#include "qcorconnection.h"
#include "qcorrequest.h"

QCorConnection::QCorConnection(QTcpSocket *socket, QObject *parent) :
  QObject(parent),
  _socket(socket),
  _currentRequest(0)
{
  connect(socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

QTcpSocket* QCorConnection::socket() const
{
  return _socket;
}

void QCorConnection::onSocketReadyRead()
{
  quint64 available = 0;
  while ((available = _socket->bytesAvailable()) > 0) {
    const QByteArray data = _socket->read(available);
  }
}

void QCorConnection::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
  case QAbstractSocket::UnconnectedState:
    deleteLater();
    break;
  }
}
