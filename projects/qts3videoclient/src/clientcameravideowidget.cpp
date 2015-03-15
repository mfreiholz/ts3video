#include "clientcameravideowidget.h"

#include <QTime>
#include <QBoxLayout>
#include <QLabel>
#include <QCameraInfo>
#include <QCamera>

#include "humblelogging/api.h"

#include "elws.h"

#include "ts3videoclient.h"
#include "cameraframegrabber.h"
#include "videowidget.h"

HUMBLE_LOGGER(HL, "client.camera");

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(TS3VideoClient *ts3vc, const QCameraInfo &cameraInfo, QWidget *parent) :
  QWidget(parent),
  _ts3vc(ts3vc)
{
  auto camera = new QCamera(cameraInfo, this);
  camera->start();

  auto grabber = new CameraFrameGrabber(this);
  auto videoWidget = new VideoWidget(VideoWidget::OpenGL_ImageWidget);
  camera->setViewfinder(grabber);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(videoWidget, 1);
  setLayout(mainLayout);

  QObject::connect(grabber, &CameraFrameGrabber::newQImage, [ts3vc, videoWidget] (const QImage &image) {
    videoWidget->setFrame(image);
  });

  QObject::connect(grabber, &CameraFrameGrabber::newQImage, [ts3vc, videoWidget](const QImage &image) {
    if (ts3vc->isReadyForStreaming()) {
      ts3vc->sendVideoFrame(image);
    }
  });

  QObject::connect(camera, &QCamera::stateChanged, [this, camera](QCamera::State state) {
    switch (state) {
    case QCamera::UnloadedState:
      break;
    case QCamera::LoadedState:
      break;
    case QCamera::ActiveState:
      break;
    }
  });
}

ClientCameraVideoWidget::~ClientCameraVideoWidget()
{
}