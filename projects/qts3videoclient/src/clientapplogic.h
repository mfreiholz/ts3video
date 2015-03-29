#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QHostAddress>

#include "ts3video.h"
#include "yuvframe.h"

#include "ts3videoclient.h"

class QWidget;
class QProgressDialog;
class ViewBase;
class ClientCameraVideoWidget;
class RemoteClientVideoWidget;

/*!
  Logic to control the GUI and connection on client-side.
 */
class ClientAppLogic : public QObject
{
  Q_OBJECT

public:
  class Options
  {
  public:
    // The address and port of the remote server.
    // The address can either be a name like "myhost.com" or and IPv4/IPv6 address.
    QString serverAddress = IFVS_SERVER_ADDRESS;
    quint16 serverPort = IFVS_SERVER_CONNECTION_PORT;
    
    // The visible username of the client.
    QString username = QString();
    QString password = QString();

    // The internal Teamspeak client- and channel-id.
    // The TS3VideoClient will automatically join the channel.
    quint64 ts3clientId = 0;
    quint64 ts3channelId = 0;
    
    // The camera's device ID to stream video.
    QString cameraDeviceId = QString();
  };

public:
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