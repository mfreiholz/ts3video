#include "ts3serverqueryconnection.h"

TS3ServerQueryClientConnection::TS3ServerQueryClientConnection(QObject* parent) :
  QObject(parent)
{

}

void TS3ServerQueryClientConnection::init()
{
  _socket = new QTcpSocket(this);
  _socket->connectToHost("127.0.0.1", 10011);
  connect(_socket, &QTcpSocket::readyRead, this, &TS3ServerQueryClientConnection::onReadyRead);
  connect(_socket, &QTcpSocket::stateChanged, this, &TS3ServerQueryClientConnection::onStateChanged);
}

void TS3ServerQueryClientConnection::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state)
  {
  case QAbstractSocket::ConnectedState:
    break;
  case QAbstractSocket::UnconnectedState:
    break;
  }
}

void TS3ServerQueryClientConnection::onReadyRead()
{
  auto available = 0;
  while ((available = _socket->bytesAvailable()) > 0)
  {
    _buffer.append(_socket->read(available));

  }
}