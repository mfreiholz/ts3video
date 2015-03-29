#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QHostAddress>

#include "yuvframe.h"

#include "ts3videoclient.h"

class QWidget;
class QProgressDialog;
class ViewBase;
class ClientCameraVideoWidget;
class RemoteClientVideoWidget;

class ClientAppLogic : public QObject
{
  Q_OBJECT

public:
  class Options
  {
  public:
    Options();
    QString serverAddress;
    quint16 serverPort;
    quint64 ts3clientId;
    quint64 ts3channelId;
    QString username;
    QString cameraDeviceId;
  };

  ClientAppLogic(const Options &opts, QObject *parent);
  ~ClientAppLogic();
  bool init();

  TS3VideoClient& ts3client();

private slots:
  void onConnected();
  void onDisconnected();
  void onError(QAbstractSocket::SocketError socketError);
  void onServerError(int code, const QString &message);
  void onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientDisconnected(const ClientEntity &client);
  void onNewVideoFrame(YuvFrameRefPtr frame, int senderId);
  void onNetworkUsageUpdated(const NetworkUsageEntity &networkUsage);

protected:
  void initGui();
  QWidget* createCameraWidget();
  void showProgress(const QString &text);
  void hideProgress();
  void showError(const QString &shortText, const QString &longText = QString());

private:
  Options _opts;
  TS3VideoClient _ts3vc;

  ViewBase *_view;
  ClientCameraVideoWidget *_cameraWidget; ///< Local user's camera widget.
  QProgressDialog *_progressBox; ///< Global progress dialog.
};

#endif