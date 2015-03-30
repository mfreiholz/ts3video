#include "clientapplogic_p.h"

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCameraInfo>
#include <QHostInfo>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

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

ClientAppLogic::ClientAppLogic(const Options &opts, QWidget *parent, Qt::WindowFlags flags) :
  QMainWindow(parent, flags),
  d(new ClientAppLogicPrivate(this))
{
  d->opts = opts;

  // Progress dialog.
  d->progressDialog = new QProgressDialog(this, 0);
  d->progressDialog->setCancelButton(nullptr);
  d->progressDialog->setAutoClose(false);
  d->progressDialog->setAutoReset(false);
  d->progressDialog->setRange(0, 0);
  d->progressDialog->setModal(true);
  d->progressDialog->setVisible(false);

  // Network connection events.
  connect(&d->ts3vc, &TS3VideoClient::connected, this, &ClientAppLogic::onConnected);
  connect(&d->ts3vc, &TS3VideoClient::disconnected, this, &ClientAppLogic::onDisconnected);
  connect(&d->ts3vc, &TS3VideoClient::error, this, &ClientAppLogic::onError);
  connect(&d->ts3vc, &TS3VideoClient::serverError, this, &ClientAppLogic::onServerError);
  connect(&d->ts3vc, &TS3VideoClient::clientJoinedChannel, this, &ClientAppLogic::onClientJoinedChannel);
  connect(&d->ts3vc, &TS3VideoClient::clientLeftChannel, this, &ClientAppLogic::onClientLeftChannel);
  connect(&d->ts3vc, &TS3VideoClient::clientDisconnected, this, &ClientAppLogic::onClientDisconnected);
  connect(&d->ts3vc, &TS3VideoClient::newVideoFrame, this, &ClientAppLogic::onNewVideoFrame);
  connect(&d->ts3vc, &TS3VideoClient::networkUsageUpdated, this, &ClientAppLogic::onNetworkUsageUpdated);
}

ClientAppLogic::~ClientAppLogic()
{
  hideProgress();
  if (d->view) {
    delete d->view;
  }
  if (d->cameraWidget) {
    d->cameraWidget->close();
    delete d->cameraWidget;
  }
}

void ClientAppLogic::init()
{
  // If the address is already an IP, we don't need a lookup.
  auto address = QHostAddress(d->opts.serverAddress);
  if (!address.isNull()) {
    showProgress(tr("Connecting to server %1:%2 (address=%3)").arg(d->opts.serverAddress).arg(d->opts.serverPort).arg(address.toString()));
    d->ts3vc.connectToHost(address, d->opts.serverPort);
    return;
  }

  // Async DNS lookup.
  showProgress(tr("DNS lookup server %1").arg(d->opts.serverAddress));
  auto watcher = new QFutureWatcher<QHostInfo>(this);
  auto future = QtConcurrent::run([this] () -> QHostInfo
  {
      auto hostInfo = QHostInfo::fromName(d->opts.serverAddress);
      return hostInfo;
  });
  QObject::connect(watcher, &QFutureWatcher<QHostInfo>::finished, [this, watcher] ()
  {
    watcher->deleteLater();
    auto hostInfo = watcher->future().result();
    if (hostInfo.error() != QHostInfo::NoError || hostInfo.addresses().isEmpty()) {
      showError(tr("DNS lookup failed"), hostInfo.errorString(), true);
      return;
    }
    auto address = hostInfo.addresses().first();
    showProgress(tr("Connecting to server %1:%2 (address=%3)").arg(d->opts.serverAddress).arg(d->opts.serverPort).arg(address.toString()));
    d->ts3vc.connectToHost(address, d->opts.serverPort);
  });
  watcher->setFuture(future);
}

TS3VideoClient& ClientAppLogic::ts3client()
{
  return d->ts3vc;
}

void ClientAppLogic::onConnected()
{
  auto ts3clientId = d->opts.ts3clientId;
  auto ts3channelId = d->opts.ts3channelId;

  // Authenticate.
  showProgress(tr("Authenticating..."));
  auto reply = d->ts3vc.auth(d->opts.username, d->opts.password);
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
    auto reply2 = d->ts3vc.joinChannel(ts3channelId);
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
  HL_INFO(HL, QString("Socket error (error=%1; message=%2)").arg(socketError).arg(d->ts3vc.socket()->errorString()).toStdString());
  showError(tr("Network socket error."), d->ts3vc.socket()->errorString());
}

void ClientAppLogic::onServerError(int code, const QString &message)
{
  HL_INFO(HL, QString("Server error (error=%1; message=%2)").arg(code).arg(message).toStdString());
  //showError(tr("Server error."),  QString("%1: %2").arg(code).arg(message));
}

void ClientAppLogic::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  if (client.id != d->ts3vc.clientEntity().id) {
    d->view->addClient(client, channel);
  }
}

void ClientAppLogic::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  HL_INFO(HL, QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
  d->view->removeClient(client, channel);
}

void ClientAppLogic::onClientDisconnected(const ClientEntity &client)
{
  HL_INFO(HL, QString("Client disconnected (client-id=%1)").arg(client.id).toStdString());
  d->view->removeClient(client, ChannelEntity());
}

void ClientAppLogic::onNewVideoFrame(YuvFrameRefPtr frame, int senderId)
{
  d->view->updateClientVideo(frame, senderId);
}

void ClientAppLogic::onNetworkUsageUpdated(const NetworkUsageEntity &networkUsage)
{
  TileViewWidget *tileView = nullptr;
  QWidget *w = nullptr;

  if (d->view && (tileView = dynamic_cast<TileViewWidget*>(d->view)) != nullptr) {
    tileView->updateNetworkUsage(networkUsage);
  }
  if (d->view && (w = dynamic_cast<QWidget*>(d->view)) != nullptr) {
    auto s = QString("Received=%1; Sent=%2; D=%3; U=%4")
      .arg(ELWS::humanReadableSize(networkUsage.bytesRead))
      .arg(ELWS::humanReadableSize(networkUsage.bytesWritten))
      .arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead))
      .arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite));
    w->setWindowTitle(s);
  }
}

void ClientAppLogic::showEvent(QShowEvent *e)
{
  QSettings settings;
  restoreGeometry(settings.value("UI/ClientApp-Geometry").toByteArray());
}

void ClientAppLogic::closeEvent(QCloseEvent *e)
{
  QSettings settings;
  settings.setValue("UI/ClientApp-Geometry", saveGeometry());
}

void ClientAppLogic::initGui()
{
  auto viewWidget = new TileViewWidget(this);
  d->view = viewWidget;
  d->view->setCameraWidget(createCameraWidget());
  setCentralWidget(viewWidget);
}

QWidget* ClientAppLogic::createCameraWidget()
{
  auto cameraInfo = QCameraInfo::defaultCamera();
  foreach (auto ci, QCameraInfo::availableCameras()) {
    if (ci.deviceName() == d->opts.cameraDeviceId) {
      cameraInfo = ci;
      break;
    }
  }
  d->cameraWidget = new ClientCameraVideoWidget(&d->ts3vc, cameraInfo, nullptr);
  return d->cameraWidget;
}

void ClientAppLogic::showProgress(const QString &text)
{
  d->progressDialog->setLabelText(text);
  d->progressDialog->setVisible(true);
}

void ClientAppLogic::hideProgress()
{
  d->progressDialog->setLabelText(QString());
  d->progressDialog->setVisible(false);
  d->progressDialog->close();
}

void ClientAppLogic::showError(const QString &shortText, const QString &longText, bool exitApp)
{
  hideProgress();

  QMessageBox box(this);
  box.setIcon(QMessageBox::Critical);
  box.addButton(QMessageBox::Ok);
  box.setText(shortText);
  box.setDetailedText(longText);
  box.setMinimumWidth(400);
  box.exec();

  if (exitApp) {
    close();
    qApp->quit();
  }
}