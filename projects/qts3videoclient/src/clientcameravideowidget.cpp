#include "clientcameravideowidget.h"

#include <QTime>
#include <QBoxLayout>
#include <QLabel>
#include <QCameraInfo>
#include <QVideoWidget>

#include "humblelogging/api.h"

#include "ts3videoclient.h"
#include "videowidget.h"

HUMBLE_LOGGER(HL, "client.camera");

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(TS3VideoClient *ts3vc, QWidget *parent) :
  QWidget(parent),
  _ts3vc(ts3vc)
{
  auto cameraInfo = QCameraInfo::defaultCamera();
  auto camera = new QCamera(cameraInfo, this);
  camera->start();

  auto grabber = new CameraFrameGrabber(this);
  auto videoWidget = new ClientVideoWidget(ClientVideoWidget::CPU);
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

///////////////////////////////////////////////////////////////////////

CameraFrameGrabber::CameraFrameGrabber(QObject *parent) :
  QAbstractVideoSurface(parent),
  _firstFrame(true)
{
}

QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
  static QList<QVideoFrame::PixelFormat> __formats;
  if (__formats.isEmpty()) {
    __formats
      << QVideoFrame::Format_ARGB32
      << QVideoFrame::Format_ARGB32_Premultiplied
      << QVideoFrame::Format_RGB32
      << QVideoFrame::Format_RGB24
      << QVideoFrame::Format_RGB565
      << QVideoFrame::Format_RGB555
      << QVideoFrame::Format_ARGB8565_Premultiplied
      << QVideoFrame::Format_BGRA32
      << QVideoFrame::Format_BGRA32_Premultiplied
      << QVideoFrame::Format_BGR32
      << QVideoFrame::Format_BGR24
      << QVideoFrame::Format_BGR565
      << QVideoFrame::Format_BGR555
      << QVideoFrame::Format_BGRA5658_Premultiplied
      << QVideoFrame::Format_AYUV444
      << QVideoFrame::Format_AYUV444_Premultiplied
      << QVideoFrame::Format_YUV444
      << QVideoFrame::Format_YUV420P
      << QVideoFrame::Format_YV12
      << QVideoFrame::Format_UYVY
      << QVideoFrame::Format_YUYV
      << QVideoFrame::Format_NV12
      << QVideoFrame::Format_NV21
      << QVideoFrame::Format_IMC1
      << QVideoFrame::Format_IMC2
      << QVideoFrame::Format_IMC3
      << QVideoFrame::Format_IMC4
      << QVideoFrame::Format_Y8
      << QVideoFrame::Format_Y16
      << QVideoFrame::Format_Jpeg
      << QVideoFrame::Format_CameraRaw
      << QVideoFrame::Format_AdobeDng;
  }
  return __formats;
}

bool CameraFrameGrabber::present(const QVideoFrame &frame)
{
  if (!frame.isValid()) {
    return false;
  }
  // Clone frame to have it as non-const.
  QVideoFrame f(frame);
  if (_firstFrame) {
    HL_INFO(HL, QString("Camera first frame (format=%1; width=%2; height=%3)").arg(f.pixelFormat()).arg(f.width()).arg(f.height())  .toStdString());
    _firstFrame = false;
  }
  // Convert video frame to QImage.
  auto imageFormat = QVideoFrame::imageFormatFromPixelFormat(f.pixelFormat());
  if (imageFormat == QImage::Format_Invalid) {
    qDebug() << QString("Invalid image format for video frame.");
    return false;
  }
  if (f.map(QAbstractVideoBuffer::ReadOnly)) {
    // Create copy via copy() or mirrored(). At least we need a copy as long as we don't directly print here.
    auto image = QImage(f.bits(), f.width(), f.height(), imageFormat).mirrored();
    emit newQImage(image);
    f.unmap();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////
