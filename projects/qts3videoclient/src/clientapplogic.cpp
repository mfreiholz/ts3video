#include "clientapplogic.h"

#include <QDebug>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

#include "qcorreply.h"
#include "qcorframe.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"
#include "elws.h"

#include "clientvideowidget.h"
#include "clientcameravideowidget.h"

///////////////////////////////////////////////////////////////////////

ClientAppLogic::ClientAppLogic(QObject *parent) :
  QObject(parent)
{
  auto serverAddress = ELWS::getArgsValue("--server-address", "127.0.0.1").toString();
  auto serverPort = ELWS::getArgsValue("--server-port", 6000).toUInt();
  auto ts3clientId = ELWS::getArgsValue("--ts3-clientid").toString();
  auto ts3channelId = ELWS::getArgsValue("--ts3-channelid").toString();

  _ts3vc.connectToHost(QHostAddress(serverAddress), serverPort);
  connect(&_ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&_ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&_ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&_ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&_ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);

  _cameraWidget = new ClientCameraVideoWidget(nullptr);
  _cameraWidget->show();
}

ClientAppLogic::~ClientAppLogic()
{
  _cameraWidget->close();
  delete _cameraWidget;
}

void ClientAppLogic::onConnected()
{
  // Authenticate.
  auto reply = _ts3vc.auth("That's my name!");
  QObject::connect(reply, &QCorReply::finished, [this, reply] () {
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
    auto reply2 = _ts3vc.joinChannel();
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
      // Extract participants and create a widget for each (skip own!).
      foreach (auto v, params["participants"].toArray()) {
        ClientEntity client;
        client.fromQJsonObject(v.toObject());
        if (client.id == _ts3vc.clientEntity().id) {
          continue;
        }
        auto w = _clientWidgets.value(client.id);
        if (!w) {
          w = createClientWidget(client);
          _clientWidgets.insert(client.id, w);
        }
        if (!w->isVisible()) {
          w->show();
        }
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
  // Close widget of client.
  auto w = _clientWidgets.take(client.id);
  if (w) {
    w->close();
    w->deleteLater();
  }
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  qDebug() << QString("Client disconnected (client-id=%1)").arg(client.id);
  // Close widget of client.
  auto w = _clientWidgets.take(client.id);
  if (w) {
    w->close();
    w->deleteLater();
  }
}

QWidget* ClientAppLogic::createClientWidget(const ClientEntity &client)
{
  auto w = new ClientVideoWidget(nullptr);
  w->setWindowTitle(QString("Client: %1").arg(client.id));
  w->show();
  return w;
}