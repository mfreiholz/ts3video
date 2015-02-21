#include "remoteclientvideowidget.h"

#include <QBoxLayout>

///////////////////////////////////////////////////////////////////////

RemoteClientVideoWidget::RemoteClientVideoWidget(QWidget *parent) :
  QFrame(parent)
{
  _videoWidget = new ClientVideoWidget(ClientVideoWidget::CPU, this);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(_videoWidget);
  setLayout(mainLayout);
}

RemoteClientVideoWidget::~RemoteClientVideoWidget()
{

}

void RemoteClientVideoWidget::setClient(const ClientEntity &client)
{
  _client = client;
  _videoWidget->setAvatar(QPixmap::fromImage(QImage(QString(":/avatar.jpg"))));
  _videoWidget->setText(QString("%1 (ID=%2)").arg(client.name).arg(client.id));
}

ClientVideoWidget* RemoteClientVideoWidget::videoWidget() const
{
  return _videoWidget;
}