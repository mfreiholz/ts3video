#include "viewbase.h"

#include <QWidget>
#include <QGraphicsDropShadowEffect>

#include "remoteclientvideowidget.h"

///////////////////////////////////////////////////////////////////////

RemoteClientVideoWidget* ViewBase::createRemoteVideoWidget(const ClientEntity &client, QWidget *parent)
{
  auto w = new RemoteClientVideoWidget(parent);
  if (client.id > 0) {
    w->setClient(client);
  }
  return w;
}

void ViewBase::addDropShadowEffect(QWidget *widget)
{
  auto dropShadow = new QGraphicsDropShadowEffect(widget);
  dropShadow->setOffset(0, 0);
  dropShadow->setBlurRadius(5);
  dropShadow->setColor(QColor(Qt::black));
  widget->setGraphicsEffect(dropShadow);
}