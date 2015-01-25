#include "ts3videoclient.h"
#include "ts3videoclient_p.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "qcorconnection.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

///////////////////////////////////////////////////////////////////////

TS3VideoClient::TS3VideoClient(QObject *parent) :
  QObject(parent),
  d_ptr(new TS3VideoClientPrivate(this))
{
  Q_D(TS3VideoClient);
  connect(d->_connection, &QCorConnection::stateChanged, this, &TS3VideoClient::onStateChanged);
  connect(d->_connection, &QCorConnection::newIncomingRequest, this, &TS3VideoClient::onNewIncomingRequest);
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
  req.setData(JsonProtocolHelper::createJsonRequest("auth", params));
  return d->_connection->sendRequest(req);
}

QCorReply* TS3VideoClient::joinChannel()
{
  Q_D(TS3VideoClient);
  QJsonObject params;
  params["channelid"] = 1;
  QCorFrame req;
  req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
  return d->_connection->sendRequest(req);
}

void TS3VideoClient::onStateChanged(QAbstractSocket::SocketState state)
{
}

void TS3VideoClient::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  Q_D(TS3VideoClient);
  qDebug() << QString("Incoming request from server (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data()));
  
  QString action;
  QJsonObject parameters;
  if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, parameters)) {
    // Invalid protocol.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(JsonProtocolHelper::createJsonResponseError(500, "Invalid protocol format."));
    d->_connection->sendResponse(res);
    return;
  }

  if (action == "notify.clientjoinedchannel") {
    ChannelEntity channelEntity;
    channelEntity.fromQJsonObject(parameters["channel"].toObject());
    ClientEntity clientEntity;
    clientEntity.fromQJsonObject(parameters["client"].toObject());
    emit clientJoinedChannel(clientEntity, channelEntity);
  }

  // Response with error.
  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(JsonProtocolHelper::createJsonResponseError(404));
  d->_connection->sendResponse(res);
}

///////////////////////////////////////////////////////////////////////

TS3VideoClientPrivate::TS3VideoClientPrivate(TS3VideoClient *owner) :
  q_ptr(owner),
  _connection(new QCorConnection(owner))
{
}