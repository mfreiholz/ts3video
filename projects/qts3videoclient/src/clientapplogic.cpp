#include "clientapplogic.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>

#include "humblelogging/api.h"

#include "qcorreply.h"
#include "qcorframe.h"

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

#include "clientcameravideowidget.h"
#include "remoteclientvideowidget.h"
#include "videocollectionwidget.h"

HUMBLE_LOGGER(HL, "client.logic");

///////////////////////////////////////////////////////////////////////

ClientAppLogic::Options::Options() :
  serverAddress(),
  serverPort(0),
  ts3clientId(0),
  ts3channelId(0),
  username()
{
}

///////////////////////////////////////////////////////////////////////

ClientAppLogic::ClientAppLogic(const Options &opts, QObject *parent) :
  QObject(parent),
  _opts(opts),
  _containerWidget(nullptr),
  _cameraWidget(nullptr),
  _progressBox(nullptr)
{
  showProgress(tr("Connecting to server %1:%2").arg(_opts.serverAddress.toString()).arg(_opts.serverPort));
  _ts3vc.connectToHost(_opts.serverAddress, _opts.serverPort);
  connect(&_ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&_ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&_ts3vc, &TS3VideoClient::error, this, &ClientAppLogic::onError);
  connect(&_ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&_ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&_ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);
  connect(&_ts3vc, &TS3VideoClient::newVideoFrame, this, &ClientAppLogic::onNewVideoFrame);
}

ClientAppLogic::~ClientAppLogic()
{
  hideProgress();

  if (_containerWidget) {
    _containerWidget->close();
    delete _containerWidget;
  }

  while (!_clientWidgets.isEmpty()) {
    auto w = _clientWidgets.take(_clientWidgets.begin().key());
    delete w;
  }

  if (_cameraWidget) {
    _cameraWidget->close();
    delete _cameraWidget;
  }
}

TS3VideoClient& ClientAppLogic::ts3client()
{
  return _ts3vc;
}

void ClientAppLogic::onConnected()
{
  auto ts3clientId = _opts.ts3clientId;
  auto ts3channelId = _opts.ts3channelId;
  auto username = _opts.username;

  // Authenticate.
  showProgress(tr("Authenticating..."));
  auto reply = _ts3vc.auth(username);
  QObject::connect(reply, &QCorReply::finished, [this, reply, ts3clientId, ts3channelId] () {
    HL_DEBUG(HL, QString("Auth answer: %1").arg(QString(reply->frame()->data())).toStdString());
    reply->deleteLater();

    int status;
    QJsonObject params;
    if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params)) {
      this->showError(reply->frame()->data());
      return;
    } else if (status != 0) {
      this->showError(reply->frame()->data());
      return;
    }

    // Join channel.
    showProgress(tr("Joining channel..."));
    auto reply2 = _ts3vc.joinChannel(ts3channelId);
    QObject::connect(reply2, &QCorReply::finished, [this, reply2] () {
      HL_DEBUG(HL, QString("Join channel answer: %1").arg(QString(reply2->frame()->data())).toStdString());
      reply2->deleteLater();
      
      int status;
      QJsonObject params;
      if (!JsonProtocolHelper::fromJsonResponse(reply2->frame()->data(), status, params)) {
        this->showError(reply2->frame()->data());
        return;
      } else if (status != 0) {
        this->showError(reply2->frame()->data());
        return;
      }
      
      // Create user interface.
      initGui();

      // Extract channel.
      ChannelEntity channel;
      channel.fromQJsonObject(params["channel"].toObject());

      // Extract participants and create widgets.
      foreach (auto v, params["participants"].toArray()) {
        ClientEntity client;
        client.fromQJsonObject(v.toObject());
        onClientJoinedChannel(client, channel);
      }
      hideProgress();
    });
  });
}

void ClientAppLogic::onDisconnected()
{
  HL_INFO(HL, QString("Disconnected").toStdString());
}

void ClientAppLogic::onError(QAbstractSocket::SocketError socketError)
{
  HL_INFO(HL, QString("Socket error (error=%1; message=%2)").arg(socketError).arg(_ts3vc.socket()->errorString()).toStdString());
  showError(tr("Network socket error."), _ts3vc.socket()->errorString());
}

void ClientAppLogic::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  auto w = _clientWidgets.value(client.id);
  if (!w) {
    w = createClientWidget(client);
    _clientWidgets.insert(client.id, w);
  }
}

void ClientAppLogic::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  deleteClientWidget(client);
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  HL_INFO(HL, QString("Client disconnected (client-id=%1)").arg(client.id).toStdString());
  deleteClientWidget(client);
}

void ClientAppLogic::onNewVideoFrame(YuvFrameRefPtr frame, int senderId)
{
  auto w = _clientWidgets.value(senderId);
  if (!w) {
    ClientEntity client;
    client.id = senderId;
    client.name = "UNKNOWN!";
    w = createClientWidget(client);
    _clientWidgets.insert(senderId, w);
  }
  w->videoWidget()->setFrame(frame);
}

void ClientAppLogic::initGui()
{
  _containerWidget = new VideoCollectionWidget(nullptr);
  _containerWidget->show();
  createCameraWidget();
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
    return;
  }
  _containerWidget->removeWidget(w);
  w->close();
  w->deleteLater();
}

void ClientAppLogic::showProgress(const QString &text)
{
  if (!_progressBox) {
    _progressBox = new QProgressDialog(nullptr);
    _progressBox->setMinimumWidth(400);
    _progressBox->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    _progressBox->setCancelButton(nullptr);
    _progressBox->setAutoClose(false);
    _progressBox->setAutoReset(false);
    _progressBox->setRange(0, 0);
  }
  _progressBox->setLabelText(text);
  _progressBox->show();
}

void ClientAppLogic::hideProgress()
{
  if (_progressBox) {
    _progressBox->hide();
  }
}

void ClientAppLogic::showError(const QString &shortText, const QString &longText)
{
  hideProgress();

  QMessageBox box(qApp->activeWindow());
  box.setIcon(QMessageBox::Critical);
  box.addButton(QMessageBox::Ok);
  box.setText(shortText);
  box.setDetailedText(longText);
  box.setMinimumWidth(400);
  box.exec();
  qApp->quit();
}