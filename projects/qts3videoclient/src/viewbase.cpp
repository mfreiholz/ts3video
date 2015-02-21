#include "viewbase.h"

#include "remoteclientvideowidget.h"

///////////////////////////////////////////////////////////////////////

RemoteClientVideoWidget* ViewBase::createVideoWidget(const ClientEntity &client, QWidget *parent)
{
  auto w = new RemoteClientVideoWidget(parent);
  w->setClient(client);
  return w;
}