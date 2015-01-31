#include "remoteclientvideowidget.h"

#include <QBoxLayout>

#include "clientvideowidget.h"

///////////////////////////////////////////////////////////////////////

RemoteClientVideoWidget::RemoteClientVideoWidget(QWidget *parent) :
  QFrame(parent)
{
  _videoWidget = new ClientVideoWidget(this);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(_videoWidget);
  setLayout(mainLayout);
}

RemoteClientVideoWidget::~RemoteClientVideoWidget()
{

}

void RemoteClientVideoWidget::setClient(const ClientEntity &&client)
{
  _client = client;
  _videoWidget->setText(client.name);
}

ClientVideoWidget* RemoteClientVideoWidget::videoWidget() const
{
  return _videoWidget;
}