#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QObject>
#include <QHash>

#include "ts3videoclient.h"

class QWidget;
class ClientCameraVideoWidget;
class RemoteClientVideoWidget;

class ClientAppLogic : public QObject
{
  Q_OBJECT

public:
  ClientAppLogic(QObject *parent);
  ~ClientAppLogic();
  TS3VideoClient& ts3client();

private slots:
  void onConnected();
  void onDisconnected();
  void onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientDisconnected(const ClientEntity &client);
  void onNewVideoFrame(const QImage &image, int senderId);

protected:
  QWidget* createCameraWidget();
  RemoteClientVideoWidget* createClientWidget(const ClientEntity &client);
  void deleteClientWidget(const ClientEntity &client);

private:
  TS3VideoClient _ts3vc;

  ClientCameraVideoWidget *_cameraWidget; ///< Local user's camera widget.
  QHash<int, RemoteClientVideoWidget*> _clientWidgets; ///< Remote client widgets.

  class VideoCollectionWidget *_containerWidget;
};

#endif