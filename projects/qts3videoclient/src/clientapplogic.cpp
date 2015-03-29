#include "clientapplogic.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCameraInfo>
#include <QHostInfo>

#include "humblelogging/api.h"

#include "qcorreply.h"
#include "qcorframe.h"

#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"
#include "networkusageentity.h"
#include "jsonprotocolhelper.h"

#include "clientcameravideowidget.h"
#include "remoteclientvideowidget.h"
#include "hangoutviewwidget.h"
#include "tileviewwidget.h"

HUMBLE_LOGGER(HL, "client.logic");

///////////////////////////////////////////////////////////////////////

ClientAppLogic::Options::Options() :
  serverAddress(),
  serverPort(0),
  ts3clientId(0),
  ts3channelId(0),
  username(),
  cameraDeviceId()
{
}

///////////////////////////////////////////////////////////////////////

ClientAppLogic::ClientAppLogic(const Options &opts, QObject *parent) :
  QObject(parent),
  _opts(opts),
  _view(nullptr),
  _cameraWidget(nullptr),
  _progressBox(nullptr)
{
  connect(&_ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&_ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&_ts3vc, &TS3VideoClient::error, this, &ClientAppLogic::onError);
  connect(&_ts3vc, &TS3VideoClient::serverError, this, &ClientAppLogic::onServerError);
  connect(&_ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&_ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&_ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);
  connect(&_ts3vc, &TS3VideoClient::newVideoFrame, this, &ClientAppLogic::onNewVideoFrame);
  connect(&_ts3vc, &TS3VideoClient::networkUsageUpdated, this, &ClientAppLogic::onNetworkUsageUpdated);
}

ClientAppLogic::~ClientAppLogic()
{
  hideProgress();
  if (_view) {
    delete _view;
  }
  if (_cameraWidget) {
    _cameraWidget->close();
    delete _cameraWidget;
  }
}

bool ClientAppLogic::init()
{
  // DNS lookup in case of named-address.
  HL_DEBUG(HL, QString("Lookup server address (address=%1)").arg(_opts.serverAddress).toStdString());
  auto address = QHostAddress(_opts.serverAddress);
  if (address.isNull()) {
    auto hostInfo = QHostInfo::fromName(_opts.serverAddress);
    if (hostInfo.addresses().size() == 0) {
      HL_ERROR(HL, QString("Can not resolve server address (address=%1)").arg(_opts.serverAddress).toStdString());
      return false;
    }
    address = hostInfo.addresses().first();
  }
  if (address.isNull()) {
    HL_ERROR(HL, QString("Invalid server address (address=%1)").arg(_opts.serverAddress).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Resolved server address (name=%1; ip=%2)").arg(_opts.serverAddress).arg(address.toString()).toStdString());

  // Connect to remote server.
  showProgress(tr("Connecting to server %1:%2 (IP=%3)").arg(_opts.serverAddress).arg(_opts.serverPort).arg(address.toString()));
  _ts3vc.connectToHost(address, _opts.serverPort);
  return true;
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
      this->showError(tr("Protocol error"), reply->frame()->data());
      return;
    } else if (status != 0) {
      this->showError(tr("Protocol error"), reply->frame()->data());
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
        this->showError(tr("Protocol error"), reply2->frame()->data());
        return;
      } else if (status != 0) {
        this->showError(tr("Protocol error"), reply2->frame()->data());
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

void ClientAppLogic::onServerError(int code, const QString &message)
{
  HL_INFO(HL, QString("Server error (error=%1; message=%2)").arg(code).arg(message).toStdString());
  //showError(tr("Server error."),  QString("%1: %2").arg(code).arg(message));
}

void ClientAppLogic::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  if (client.id != _ts3vc.clientEntity().id) {
    _view->addClient(client, channel);
  }
}

void ClientAppLogic::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  _view->removeClient(client, channel);
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  HL_INFO(HL, QString("Client disconnected (client-id=%1)").arg(client.id).toStdString());
  _view->removeClient(client, ChannelEntity());
}

void ClientAppLogic::onNewVideoFrame(YuvFrameRefPtr frame, int senderId)
{
  _view->updateClientVideo(frame, senderId);
}

void ClientAppLogic::onNetworkUsageUpdated(const NetworkUsageEntity &networkUsage)
{
  TileViewWidget *tileView = nullptr;
  QWidget *w = nullptr;

  if (_view && (tileView = dynamic_cast<TileViewWidget*>(_view)) != nullptr) {
    tileView->updateNetworkUsage(networkUsage);
  }
  if (_view && (w = dynamic_cast<QWidget*>(_view)) != nullptr) {
    auto s = QString("Received=%1; Sent=%2; D=%3; U=%4")
      .arg(ELWS::humanReadableSize(networkUsage.bytesRead))
      .arg(ELWS::humanReadableSize(networkUsage.bytesWritten))
      .arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead))
      .arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite));
    w->setWindowTitle(s);
  }
}

void ClientAppLogic::initGui()
{
  //_view = new HangoutViewWidget(nullptr);
  _view = new TileViewWidget(nullptr);
  _view->setCameraWidget(createCameraWidget());
}

QWidget* ClientAppLogic::createCameraWidget()
{
  auto cameraInfo = QCameraInfo::defaultCamera();
  foreach (auto ci, QCameraInfo::availableCameras()) {
    if (ci.deviceName() == _opts.cameraDeviceId) {
      cameraInfo = ci;
      break;
    }
  }
  _cameraWidget = new ClientCameraVideoWidget(&_ts3vc, cameraInfo, nullptr);
  return _cameraWidget;
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