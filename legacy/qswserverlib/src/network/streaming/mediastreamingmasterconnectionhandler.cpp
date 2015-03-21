#include "QTimerEvent"
#include "QTcpSocket"
#include "QDataStream"
#include "humblelogging/api.h"
#include "mediastreamingmasterconnectionhandler_p.h"

HUMBLE_LOGGER(HL, "server.streaming.masterconnection");

///////////////////////////////////////////////////////////////////////////////
// MediaStreamingMasterConnectionHandler
///////////////////////////////////////////////////////////////////////////////

MediaStreamingMasterConnectionHandler::MediaStreamingMasterConnectionHandler(MediaStreamingServer *server, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_server = server;
}

MediaStreamingMasterConnectionHandler::~MediaStreamingMasterConnectionHandler()
{
  delete d;
}

void MediaStreamingMasterConnectionHandler::startup(const StartOptions &opts)
{
  d->_opts = opts;
  d->_shutdown = false;
  d->_socket->connectToHost(opts.address, opts.port);
}

void MediaStreamingMasterConnectionHandler::shutdown()
{
  d->_shutdown = true;
  d->_socket->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MediaStreamingMasterConnectionHandler::Private::Private(MediaStreamingMasterConnectionHandler *owner) :
  QObject(0),
  _owner(owner),
  _server(0),
  _socket(new QTcpSocket(this)),
  _shutdown(false),
  _updateTimerId(-1)
{
  connect(_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(_socket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
}

MediaStreamingMasterConnectionHandler::Private::~Private()
{
  delete _socket;
}

void MediaStreamingMasterConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Connected to master server (address=%1; port=%2)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).toStdString());
      _updateTimerId = startTimer(5000);
      break;
    case QAbstractSocket::UnconnectedState:
      if (!_shutdown) {
        HL_ERROR(HL, QString("Lost connection to master server (error=%1)").arg(_socket->errorString()).toStdString());
        _socket->connectToHost(_opts.address, _opts.port);
      }
      if (_updateTimerId != -1) {
        killTimer(_updateTimerId);
        _updateTimerId = -1;
      }
      emit _owner->disconnected();
      break;
  }
}

void MediaStreamingMasterConnectionHandler::Private::onSocketReadyRead()
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

void MediaStreamingMasterConnectionHandler::Private::processRequest(TCP::Request &request)
{
  HL_DEBUG(HL, "New request");
  QDataStream in(request.body);

  QString action;
  in >> action;
  if (action == "update-clients") {
    UdpDataReceiverMap map;
    in >> map;
    emit _owner->clientsUpdated(map);
  }
}

void MediaStreamingMasterConnectionHandler::Private::timerEvent(QTimerEvent *ev)
{
  if (ev->timerId() == _updateTimerId) {
    sendUpdate();
  }
}

void MediaStreamingMasterConnectionHandler::Private::sendUpdate()
{
  HL_DEBUG(HL, QString("Send node's status to master server").toStdString());

  QByteArray body;
  QDataStream out(&body, QIODevice::WriteOnly);
  out << QString("status-update");
  out << _opts.externalStreamingAddress.toString();
  out << (quint16) _opts.externalStreamingPort;
  out << (quint64) 0; // current used bandwidth
  out << (quint64) 0; // available bandwidth

  TCP::Request request;
  request.setBody(body);
  _socket->write(_protocol.serialize(request));
}