#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "qcorconnection.h"

///////////////////////////////////////////////////////////////////////

QByteArray createJsonRequest(const QString &action, const QJsonObject &parameters)
{
  QJsonObject root;
  root["action"] = action;
  root["parameters"] = parameters;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

///////////////////////////////////////////////////////////////////////

TS3VideoClient::TS3VideoClient(QObject *parent) :
  QObject(parent),
  d_ptr(new TS3VideoClientPrivate(this))
{
  Q_D(TS3VideoClient);
  connect(d->_connection, &QCorConnection::stateChanged, this, &TS3VideoClient::onStateChanged);
  connect(d->_connection, &QCorConnection::stateChanged, this, &TS3VideoClient::stateChanged);
}

TS3VideoClient::~TS3VideoClient()
{
  Q_D(TS3VideoClient);
  delete d->_connection;
}

void TS3VideoClient::connectToHost(const QHostAddress &address, qint16 port)
{
  Q_D(TS3VideoClient);
  d->_connection->connectTo(address, port);
}

QCorReply* TS3VideoClient::auth()
{
  Q_D(TS3VideoClient);
  QJsonObject params;
  params["version"] = 1;
  params["username"] = QString("UsernameHere");
  QCorFrame req;
  req.setData(createJsonRequest("auth", params));
  return d->_connection->sendRequest(req);
}

QCorReply* TS3VideoClient::joinChannel()
{
  Q_D(TS3VideoClient);
  QJsonObject params;
  params["channelid"] = 1;
  QCorFrame req;
  req.setData(createJsonRequest("joinchannel", params));
  return d->_connection->sendRequest(req);
}

void TS3VideoClient::onStateChanged(QAbstractSocket::SocketState state)
{
  qDebug() << QString("Connection state changed (state=%1)").arg(state);
}

void TS3VideoClient::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  qDebug() << QString("Incoming request from server (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data()));
}

///////////////////////////////////////////////////////////////////////

TS3VideoClientPrivate::TS3VideoClientPrivate(TS3VideoClient *owner) :
  q_ptr(owner),
  _connection(new QCorConnection(owner))
{
}