#include "clientcameravideowidget.h"

#include <QBoxLayout>
#include <QLabel>

#include <QCameraInfo>

#include <QVideoWidget>

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(QWidget *parent) :
  QWidget(parent)
{
  auto videoWidget = new QVideoWidget();
  videoWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
  videoWidget->show();

  auto cameraInfo = QCameraInfo::defaultCamera();
  auto camera = new QCamera(cameraInfo, this);
  camera->setViewfinder(videoWidget);
  camera->start();

  auto cameraLabel = new QLabel();
  cameraLabel->setText(tr("Camera: %1").arg(cameraInfo.description()));

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->addWidget(videoWidget, 1);
  mainLayout->addWidget(cameraLabel, 0);
  setLayout(mainLayout);
}

ClientCameraVideoWidget::~ClientCameraVideoWidget()
{

}