#include "QTcpSocket"
#include "QHostAddress"
#include "humblelogging/api.h"
#include "controlconnectionhandler_p.h"
#include "controlserver_p.h"
#include "controlapi.h"
#include "module/controlmodule.h"

HUMBLE_LOGGER(HL, "server.control.connection");

///////////////////////////////////////////////////////////////////////////////
// ControlConnectionHandler
///////////////////////////////////////////////////////////////////////////////

ControlConnectionHandler::ControlConnectionHandler(ControlServer *server, QTcpSocket *socket, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_server = server;
  d->_conn = new QCorConnection(d);
  d->_conn->connectWith(socket);
  d->onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(d->_conn, SIGNAL(stateChanged(QAbstractSocket::SocketState)), d, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(d->_conn, SIGNAL(newIncomingRequest(QCorFrameRefPtr)), d, SLOT(onNewIncomingRequest(QCorFrameRefPtr)));
}

ControlConnectionHandler::~ControlConnectionHandler()
{
  delete d;
}

ControlServer* ControlConnectionHandler::getServer() const
{
  return d->_server;
}

QTcpSocket* ControlConnectionHandler::getSocket() const
{
  return d->_conn->socket();
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

ControlConnectionHandler::Private::Private(ControlConnectionHandler *owner) :
  QObject(owner),
  _owner(owner),
  _server(0),
  _conn(0)
{
}

ControlConnectionHandler::Private::~Private()
{
  delete _conn;
}

void ControlConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  QTcpSocket *socket = _conn->socket();
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Client connected (address=%1; port=%2)").arg(socket->peerAddress().toString()).arg(socket->peerPort()).toStdString());
      break;
    case QAbstractSocket::UnconnectedState:
      HL_ERROR(HL, QString("Client disconnected (address=%1; port=%2; error=%3)").arg(socket->peerAddress().toString()).arg(socket->peerPort()).arg(socket->errorString()).toStdString());
      emit _owner->disconnected();
      break;
  }
}

void ControlConnectionHandler::Private::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  HL_DEBUG(HL, "New request");

  ControlProtocol::ModuleActionRequest mreq;
  if (!mreq.fromData(frame->data())) {
    HL_ERROR(HL, "Request is too small to fit in ModuleActionRequest");
    return;
  }
  HL_DEBUG(HL, QString("Module action request (module=%1; action=%2)").arg(mreq.module).arg(mreq.action).toStdString());

  // Find module.
  const QString moduleId(mreq.module);
  const QString actionId(mreq.action);
  ControlModule *module = _server->d->_id2module.value(moduleId);
  if (!module) {
    HL_ERROR(HL, QString("Unknown module (module=%1; action=%1)").arg(moduleId).toStdString());
    return;
  }

  // Execute module's action.
  ControlModule::Action a;
  a.action = actionId;
  a.data = frame->data().mid(ControlProtocol::ModuleActionRequest::MIN_SIZE);
  const ControlModule::Result result = module->handleAction(a);
  if (!result.errorMessage.isEmpty()) {
    HL_ERROR(HL, QString("Action failed (error=%1)").arg(result.errorMessage).toStdString());
    return;
  }

  // Send response.
  QCorFrame res;
  res.setCorrelationId(frame->correlationId());
  res.setData(result.data);
  _conn->sendResponse(res);
}

/*void ControlConnectionHandler::Private::processRequest(const TCP::Request &request)
{
  switch (request.header.type) {

    // Handle request from client.
    case TCP::Request::Header::REQ: {

      ControlProtocol::ModuleActionRequest mreq;
      if (!mreq.fromData(request.body)) {
        HL_ERROR(HL, "Request is too small to fit in ModuleActionRequest");
        return;
      }
      HL_DEBUG(HL, QString("Module action request (module=%1; action=%2)").arg(mreq.module).arg(mreq.action).toStdString());

      // Find module.
      const QString moduleId(mreq.module);
      const QString actionId(mreq.action);
      ControlModule *module = _server->d->_id2module.value(moduleId);
      if (!module) {
        HL_ERROR(HL, QString("Unknown module (module=%1; action=%1)").arg(moduleId).toStdString());
        return;
      }

      // Execute module's action.
      ControlModule::Action a;
      a.action = actionId;
      a.data = request.body.mid(ControlProtocol::ModuleActionRequest::MIN_SIZE);
      const ControlModule::Result result = module->handleAction(a);
      if (!result.errorMessage.isEmpty()) {
        HL_ERROR(HL, QString("Action failed (error=%1)").arg(result.errorMessage).toStdString());
        return;
      }

      // Send response.
      TCP::Request response;
      response.initResponseByRequest(request);
      response.setBody(result.data);
      if (_socket->write(_protocol.serialize(response)) == -1) {
        HL_ERROR(HL, "Can not write data to socket");
        return;
      }
      break;
    }

    // Handle the response from client of a previous request.
    case TCP::Request::Header::RESP: {
      HL_DEBUG(HL, "Response from client");
      break;
    }

  }
}*/