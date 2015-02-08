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
class VideoCollectionWidget;
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
    QHostAddress serverAddress;
    quint16 serverPort;
    quint64 ts3clientId;
    quint64 ts3channelId;
    QString username;
  };

  ClientAppLogic(const Options &opts, QObject *parent);
  ~ClientAppLogic();
  TS3VideoClient& ts3client();

private slots:
  void onConnected();
  void onDisconnected();
  void onError(QAbstractSocket::SocketError socketError);
  void onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientDisconnected(const ClientEntity &client);
  void onNewVideoFrame(YuvFrameRefPtr frame, int senderId);

protected:
  void initGui();
  QWidget* createCameraWidget();
  RemoteClientVideoWidget* createClientWidget(const ClientEntity &client);
  void deleteClientWidget(const ClientEntity &client);
  void showProgress(const QString &text);
  void hideProgress();
  void showError(const QString &shortText, const QString &longText = QString());

private:
  Options _opts;
  TS3VideoClient _ts3vc;

  VideoCollectionWidget *_containerWidget;
  ClientCameraVideoWidget *_cameraWidget; ///< Local user's camera widget.
  QHash<int, RemoteClientVideoWidget*> _clientWidgets; ///< Remote client widgets.

  QProgressDialog *_progressBox;
};

#endif