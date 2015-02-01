#ifndef REMOTECLIENTVIDEOWIDGET_H
#define REMOTECLIENTVIDEOWIDGET_H

#include <QFrame>

#include "cliententity.h"

#include "clientvideowidget.h"

class RemoteClientVideoWidget : public QFrame
{
  Q_OBJECT

public:
  RemoteClientVideoWidget(QWidget *parent);
  ~RemoteClientVideoWidget();
  void setClient(const ClientEntity &client);
  ClientVideoWidget* videoWidget() const;

private:
  ClientEntity _client;
  ClientVideoWidget *_videoWidget;
};

#endif