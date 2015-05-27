#ifndef VIEWBASE_H
#define VIEWBASE_H

#include "yuvframe.h"

class QWidget;
class RemoteClientVideoWidget;
class ClientEntity;
class ChannelEntity;
class NetworkUsageEntity;
class ClientListModel;

class ViewBase
{
public:
  ViewBase() {}
  virtual ~ViewBase() {};
  virtual void setClientListModel(ClientListModel *model) {}
  virtual void setCameraWidget(QWidget *w) = 0;
  virtual void addClient(const ClientEntity &client, const ChannelEntity &channel) = 0;
  virtual void removeClient(const ClientEntity &client, const ChannelEntity &channel) = 0;
  virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId) = 0;

  static RemoteClientVideoWidget* createRemoteVideoWidget(const ClientEntity &client, QWidget *parent = nullptr);
  static void addDropShadowEffect(QWidget *widget);
};

#endif