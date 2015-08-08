#include "ts3serverqueryconnection.h"

TS3ServerQueryClientConnection::TS3ServerQueryClientConnection(QObject* parent) :
  QObject(parent),
  _address(QHostAddress::LocalHost),
  _port(10011),
  _virtualServerPort(9987),
  _clientLoginName("serveradmin"),
  _clientLoginPassword("TiHxQDHt")
{
}

void TS3ServerQueryClientConnection::init()
{
  _socket = new QTcpSocket(this);
  connect(_socket, &QTcpSocket::connected, this, &TS3ServerQueryClientConnection::onSocketConnected);
  connect(_socket, &QTcpSocket::disconnected, this, &TS3ServerQueryClientConnection::onSocketDisconnected);
  connect(_socket, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &TS3ServerQueryClientConnection::onSocketError);
  connect(_socket, &QTcpSocket::readyRead, this, &TS3ServerQueryClientConnection::onSocketReadyRead);
  _socket->connectToHost(_address, _port);
}

void TS3ServerQueryClientConnection::onSocketConnected()
{
}

void TS3ServerQueryClientConnection::onSocketDisconnected()
{
}

void TS3ServerQueryClientConnection::onSocketError(QAbstractSocket::SocketError err)
{
}

void TS3ServerQueryClientConnection::onSocketReadyRead()
{
  auto available = 0;
  while ((available = _socket->bytesAvailable()) > 0)
  {
    _buffer.append(_socket->read(available));

  }
}