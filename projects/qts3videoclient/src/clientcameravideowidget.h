#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QWidget>

class QCameraInfo;
class TS3VideoClient;

/**
 */
class ClientCameraVideoWidget : public QWidget
{
  Q_OBJECT

public:
  ClientCameraVideoWidget(TS3VideoClient *ts3vc, const QCameraInfo &cameraInfo, QWidget *parent);
  ~ClientCameraVideoWidget();

private:
  TS3VideoClient *_ts3vc;
};

/**
 */
#include <QImage>
#include <QVideoFrame>
#include <QAbstractVideoSurface>
class CameraFrameGrabber : public QAbstractVideoSurface
{
  Q_OBJECT

public:
  CameraFrameGrabber(QObject *parent);
  QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
  bool present(const QVideoFrame &frame);

signals:
  void newQImage(const QImage &image);

private:
  bool _firstFrame;
  QSize _targetSize;

  QRect _imageRect;
  QPoint _imageOffset;
};

#endif