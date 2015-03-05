#ifndef REMOTECLIENTVIDEOWIDGET_H
#define REMOTECLIENTVIDEOWIDGET_H

#include <QFrame>

#include "cliententity.h"

#include "videowidget.h"

class RemoteClientVideoWidget : public QFrame
{
  Q_OBJECT

public:
  RemoteClientVideoWidget(QWidget *parent);
  ~RemoteClientVideoWidget();
  void setClient(const ClientEntity &client);
  VideoWidget* videoWidget() const;

private:
  ClientEntity _client;
  VideoWidget *_videoWidget;
};

#endif