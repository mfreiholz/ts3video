#ifndef VIEWBASE_H
#define VIEWBASE_H

#include "yuvframe.h"

class QWidget;
class RemoteClientVideoWidget;
class ClientEntity;
class ChannelEntity;

class ViewBase
{
public:
  ViewBase() {}
  virtual ~ViewBase() {};
  virtual void setCameraWidget(QWidget *w) = 0;
  virtual void addClient(const ClientEntity &client, const ChannelEntity &channel) = 0;
  virtual void removeClient(const ClientEntity &client, const ChannelEntity &channel) = 0;
  virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId) = 0;

protected:
  virtual RemoteClientVideoWidget* createVideoWidget(const ClientEntity &client, QWidget *parent = nullptr);
};

#endif