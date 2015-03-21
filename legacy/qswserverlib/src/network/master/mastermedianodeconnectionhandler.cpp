#include <limits.h>
#include "QTcpSocket"
#include "QHostAddress"
#include "QDataStream"
#include "humblelogging/api.h"
#include "mastermedianodeconnectionhandler_p.h"

HUMBLE_LOGGER(HL, "server.master.medianodeconnection");

///////////////////////////////////////////////////////////////////////////////
// MasterMediaNodeConnectionHandler
///////////////////////////////////////////////////////////////////////////////

MasterMediaNodeConnectionHandler::MasterMediaNodeConnectionHandler(MasterServer *server, QTcpSocket *socket, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_server = server;
  d->_socket = socket;
  d->onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), d, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(socket, SIGNAL(readyRead()), d, SLOT(onSocketReadyRead()));
}

MasterMediaNodeConnectionHandler::~MasterMediaNodeConnectionHandler()
{
  delete d;
}

QTcpSocket* MasterMediaNodeConnectionHandler::getSocket() const
{
  return d->_socket;
}

const MasterMediaNodeConnectionHandler::NodeStatus& MasterMediaNodeConnectionHandler::getNodeStatus() const
{
  return d->_nodeStatus;
}

void MasterMediaNodeConnectionHandler::sendUpdateClients(const UdpDataReceiverMap &map)
{
  QByteArray body;
  QDataStream out(&body, QIODevice::WriteOnly);
  out << "update-clients";
  out << map;

  TCP::Request request;
  request.setBody(body);
  d->_socket->write(d->_protocol.serialize(request));
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MasterMediaNodeConnectionHandler::Private::Private(MasterMediaNodeConnectionHandler *owner) :
  QObject(0),
  _owner(owner),
  _server(0),
  _socket(0)
{
}

MasterMediaNodeConnectionHandler::Private::~Private()
{
  delete _socket;
}

void MasterMediaNodeConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Media node connected (address=%1; port=%2)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).toStdString());
      break;
    case QAbstractSocket::UnconnectedState:
      HL_ERROR(HL, QString("Media node disconnected (address=%1; port=%2; error=%3)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).arg(_socket->errorString()).toStdString());
      emit _owner->disconnected();
      break;
  }
}

void MasterMediaNodeConnectionHandler::Private::onSocketReadyRead()
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

void MasterMediaNodeConnectionHandler::Private::processRequest(TCP::Request &request)
{
  HL_DEBUG(HL, "New request");
  QDataStream in(request.body);

  QString action;
  in >> action;
  if (action == "status-update") {
    MasterMediaNodeConnectionHandler::NodeStatus status;
    status.lastUpdate = QDateTime::currentDateTimeUtc();
    in >> status.externalStreamingAddress;
    in >> status.externalStreamingPort;
    in >> status.currentBandwidth;
    in >> status.availableBandwidth;
    _nodeStatus = status;
    HL_DEBUG(HL, QString("Status update (extaddr=%1; extport=%2; currbw=%3; avaibw=%4)").arg(status.externalStreamingAddress).arg(status.externalStreamingPort).arg(status.currentBandwidth).arg(status.availableBandwidth).toStdString());
  }
}