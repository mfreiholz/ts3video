#ifndef TILEVIEWWIDGET_H
#define TILEVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "viewbase.h"

class TileViewWidgetPrivate;
class TileViewWidget : public QWidget, public ViewBase
{
  Q_OBJECT
  QScopedPointer<TileViewWidgetPrivate> d;

public:
  TileViewWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
  virtual ~TileViewWidget();

  virtual void setCameraWidget(QWidget *w);
  virtual void addClient(const ClientEntity &client, const ChannelEntity &channel);
  virtual void removeClient(const ClientEntity &client, const ChannelEntity &channel);
  virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId);
};

#endif