#ifndef HANGOUTVIEWWIDGET_H
#define HANGOUTVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "viewbase.h"

class HangoutViewWidgetPrivate;
class HangoutViewWidget : public QWidget, public ViewBase
{
  Q_OBJECT
  QScopedPointer<HangoutViewWidgetPrivate> d;

public:
  HangoutViewWidget(QWidget *parent);
  virtual ~HangoutViewWidget();
  
  // From ViewBase.
  virtual void setCameraWidget(QWidget *w) override;
  virtual void addClient(const ClientEntity &client, const ChannelEntity &channel) override;
  virtual void removeClient(const ClientEntity &client, const ChannelEntity &channel) override;
  virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId) override;

protected:
  void resizeEvent(QResizeEvent *);
};

#endif