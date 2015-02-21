#include "viewbase.h"

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