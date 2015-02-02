#include "clientapplogic.h"

#include <QDebug>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include <QApplication>

#include "qcorreply.h"
#include "qcorframe.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"
#include "elws.h"

#include "clientcameravideowidget.h"
#include "remoteclientvideowidget.h"
#include "videocollectionwidget.h"

///////////////////////////////////////////////////////////////////////

ClientAppLogic::ClientAppLogic(QObject *parent) :
  QObject(parent)
{
  auto serverAddress = ELWS::getArgsValue("--server-address", "127.0.0.1").toString();
  auto serverPort = ELWS::getArgsValue("--server-port", 6000).toUInt();

  _ts3vc.connectToHost(QHostAddress(serverAddress), serverPort);
  connect(&_ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&_ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&_ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&_ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&_ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);
  connect(&_ts3vc, &TS3VideoClient::newVideoFrame, this, &ClientAppLogic::onNewVideoFrame);

  _containerWidget = new VideoCollectionWidget(nullptr);
  _containerWidget->show();

  createCameraWidget();
}

ClientAppLogic::~ClientAppLogic()
{
  _cameraWidget->close();
  delete _cameraWidget;
}

TS3VideoClient& ClientAppLogic::ts3client()
{
  return _ts3vc;
}

void ClientAppLogic::onConnected()
{
  auto ts3clientId = ELWS::getArgsValue("--ts3-clientid", 0).toUInt();
  auto ts3channelId = ELWS::getArgsValue("--ts3-channelid", 0).toUInt();
  auto username = ELWS::getArgsValue("--username", ELWS::getUserName()).toString();

  // Authenticate.
  auto reply = _ts3vc.auth(username);
  QObject::connect(reply, &QCorReply::finished, [this, reply, ts3clientId, ts3channelId] () {
    qDebug() << QString("Auth answer: %1").arg(QString(reply->frame()->data()));
    reply->deleteLater();

    int status;
    QJsonObject params;
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params)) {
      // TODO Internal error.
      return;
    } else if (status != 0) {
      // TODO Auth error.
      return;
    }

    // Join channel.
    auto reply2 = _ts3vc.joinChannel(ts3channelId);
    QObject::connect(reply2, &QCorReply::finished, [this, reply2] () {
      qDebug() << QString("Join channel answer: %1").arg(QString(reply2->frame()->data()));
      reply2->deleteLater();
      
      int status;
      QJsonObject params;
      if (!JsonProtocolHelper::fromJsonResponse(reply2->frame()->data(), status, params)) {
        // TODO Internal error.
        return;
      } else if (status != 0) {
        // TODO Can not join channel error.
        return;
      }
      
      // Extract channel.
      ChannelEntity channel;
      channel.fromQJsonObject(params["channel"].toObject());

      // Extract participants and create widgets.
      foreach (auto v, params["participants"].toArray()) {
        ClientEntity client;
        client.fromQJsonObject(v.toObject());
        onClientJoinedChannel(client, channel);
      }
    });
  });
}

void ClientAppLogic::onDisconnected()
{
  qDebug() << QString("Disconnected...");
  //qApp->quit();
}

void ClientAppLogic::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  qDebug() << QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id);
  // Create new widget for client.
  auto w = _clientWidgets.value(client.id);
  if (!w) {
    w = createClientWidget(client);
    _clientWidgets.insert(client.id, w);
  }
  if (!w->isVisible()) {
    w->show();
  }
}

void ClientAppLogic::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  qDebug() << QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id);
  deleteClientWidget(client);
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  qDebug() << QString("Client disconnected (client-id=%1)").arg(client.id);
  deleteClientWidget(client);
}

void ClientAppLogic::onNewVideoFrame(const QImage &image, int senderId)
{
  auto w = _clientWidgets.value(senderId);
  if (!w) {
    ClientEntity client;
    client.id = senderId;
    client.name = "UNKNOWN!";
    w = createClientWidget(client);
    _clientWidgets.insert(senderId, w);
  }
  w->videoWidget()->setImage(image);
}

QWidget* ClientAppLogic::createCameraWidget()
{
  _cameraWidget = new ClientCameraVideoWidget(&_ts3vc, nullptr);
  _containerWidget->addWidget(_cameraWidget);
  return _cameraWidget;
}

RemoteClientVideoWidget* ClientAppLogic::createClientWidget(const ClientEntity &client)
{
  auto w = new RemoteClientVideoWidget(nullptr);
  w->setClient(client);
  _containerWidget->addWidget(w);
  return w;
}

void ClientAppLogic::deleteClientWidget(const ClientEntity &client)
{
  auto w = _clientWidgets.take(client.id);
  if (!w) {
    qDebug() << QString("Trying to delete a non existing widget (client-id=%1)").arg(client.id);
    return;
  }
  _containerWidget->removeWidget(w);
  w->close();
  w->deleteLater();
}