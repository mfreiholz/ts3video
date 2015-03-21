#include "QTcpSocket"
#include "QHostAddress"
#include "humblelogging/api.h"
#include "masterclientnodeconnectionhandler_p.h"

HUMBLE_LOGGER(HL, "server.master.clientnodeconnection");

///////////////////////////////////////////////////////////////////////////////
// MasterClientNodeConnectionHandler
///////////////////////////////////////////////////////////////////////////////

MasterClientNodeConnectionHandler::MasterClientNodeConnectionHandler(MasterServer *server, QTcpSocket *socket, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_server = server;
  d->_socket = socket;
  d->onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), d, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(socket, SIGNAL(readyRead()), d, SLOT(onSocketReadyRead()));
}

MasterClientNodeConnectionHandler::~MasterClientNodeConnectionHandler()
{
  delete d;
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MasterClientNodeConnectionHandler::Private::Private(MasterClientNodeConnectionHandler *owner) :
  QObject(owner),
  _owner(owner),
  _server(0),
  _socket(0)
{
}

MasterClientNodeConnectionHandler::Private::~Private()
{
  delete _socket;
}

void MasterClientNodeConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Client node connected (address=%1; port=%2)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).toStdString());
      break;
    case QAbstractSocket::UnconnectedState:
      HL_ERROR(HL, QString("Client node disconnected (address=%1; port=%2; error=%3)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).arg(_socket->errorString()).toStdString());
      emit _owner->disconnected();
      break;
  }
}

void MasterClientNodeConnectionHandler::Private::onSocketReadyRead()
{
  qint64 available = 0;
  while ((available = _socket->bytesAvailable()) > 0) {
    const QByteArray data = _socket->read(available);
    _protocol.append(data);
  }

  TCP::Request *request = 0;
  while ((request = _protocol.next()) != 0) {
    processRequest(*request);
    delete request;
  }
}

void MasterClientNodeConnectionHandler::Private::processRequest(TCP::Request &request)
{
  HL_DEBUG(HL, "New request");
}