#include "QTcpSocket"
#include "QHostAddress"
#include "humblelogging/api.h"
#include "controltomasterconnectionhandler_p.h"

HUMBLE_LOGGER(HL, "server.control.masterconnection");

///////////////////////////////////////////////////////////////////////////////
// ControlToMasterConnectionHandler
///////////////////////////////////////////////////////////////////////////////

ControlToMasterConnectionHandler::ControlToMasterConnectionHandler(ControlServer *server, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_server = server;
}

ControlToMasterConnectionHandler::~ControlToMasterConnectionHandler()
{
  delete d;
}

void ControlToMasterConnectionHandler::startup(const StartupOptions &opts)
{
  d->_opts = opts;
  d->_shutdown = false;
  d->_conn->connectTo(QHostAddress(opts.address), opts.port);
}

void ControlToMasterConnectionHandler::shutdown()
{
  d->_shutdown = true;
  d->_conn->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

ControlToMasterConnectionHandler::Private::Private(ControlToMasterConnectionHandler *owner) :
  QObject(owner),
  _owner(owner),
  _server(0),
  _conn(new QCorConnection(this)),
  _shutdown(false)
{
  connect(_conn, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(_conn, SIGNAL(newIncomingRequest(QCorFrameRefPtr)), SLOT(onNewIncomingRequest(QCorFrameRefPtr)));
}

ControlToMasterConnectionHandler::Private::~Private()
{
  delete _conn;
}

void ControlToMasterConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  QTcpSocket *socket = _conn->socket();
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Connected to master server (address=%1; port=%2)").arg(socket->peerAddress().toString()).arg(socket->peerPort()).toStdString());
      break;
    case QAbstractSocket::UnconnectedState:
      if (!_shutdown) {
        HL_ERROR(HL, QString("Lost connection to master server (error=%1)").arg(socket->errorString()).toStdString());
        _owner->startup(_opts);
      }
      emit _owner->disconnected();
      break;
  }
}

void ControlToMasterConnectionHandler::Private::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  HL_DEBUG(HL, "New request");
}