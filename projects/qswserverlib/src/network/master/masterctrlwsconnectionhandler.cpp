#include "QWebSocket"
#include "QJsonDocument"
#include "QJsonValue"
#include "QJsonObject"
#include "humblelogging/api.h"
#include "masterctrlwsconnectionhandler_p.h"
#include "masterserver_p.h"

HUMBLE_LOGGER(HL, "server.master.ctrlwsconnection");

///////////////////////////////////////////////////////////////////////////////
// MasterCtrlWsConnectionHandler
///////////////////////////////////////////////////////////////////////////////

MasterCtrlWsConnectionHandler::MasterCtrlWsConnectionHandler(MasterServer *master, QWebSocket *socket, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_master = master;
  d->_socket = socket;
  d->onSocketStateChanged(QAbstractSocket::ConnectedState);
  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), d, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
  connect(socket, SIGNAL(textMessageReceived(const QString &)), d, SLOT(onTextMessageReceived(const QString &)));
}

MasterCtrlWsConnectionHandler::~MasterCtrlWsConnectionHandler()
{
  delete d;
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MasterCtrlWsConnectionHandler::Private::Private(MasterCtrlWsConnectionHandler *owner) :
  QObject(owner),
  _owner(owner),
  _master(0),
  _socket(0)
{
}

MasterCtrlWsConnectionHandler::Private::~Private()
{
  delete _socket;
}

void MasterCtrlWsConnectionHandler::Private::onSocketStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::ConnectedState:
      HL_INFO(HL, QString("Control WS connected (address=%1; port=%2)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).toStdString());
      break;
    case QAbstractSocket::UnconnectedState:
      HL_ERROR(HL, QString("Control WS disconnected (address=%1; port=%2; error=%3)").arg(_socket->peerAddress().toString()).arg(_socket->peerPort()).arg(_socket->errorString()).toStdString());
      emit _owner->disconnected();
      break;
  }
}

void MasterCtrlWsConnectionHandler::Private::onTextMessageReceived(const QString &message)
{
  HL_DEBUG(HL, QString("New text message: %1").arg(message).toStdString());

  QJsonParseError error;
  QJsonDocument doc(QJsonDocument::fromJson(message.toUtf8(), &error));
  if (doc.isNull() || error.error != QJsonParseError::NoError) {
    HL_ERROR(HL, QString("Can not parse JSON (error=%1)").arg(error.errorString()).toStdString());
    return;
  }

  QJsonObject root = doc.object();
  const QString action = root.value("action").toString();
  if (action == "status") {
    sendStatus();
  }
}

void MasterCtrlWsConnectionHandler::Private::sendStatus()
{
  QJsonObject root;
  root.insert("action", "status");

  QJsonObject status;
  status.insert("connectedmedianodes", _master->d->_mediaNodeConnections.size());
  status.insert("connectedclientnodes", _master->d->_clientNodeConnections.size());
  status.insert("connectedctrlws", _master->d->_ctrlWsConnections.size());
  root.insert("status", status);

  QJsonDocument doc;
  doc.setObject(root);
  _socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}