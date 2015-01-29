#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QWidget>

/**
 */
class ClientCameraVideoWidget : public QWidget
{
  Q_OBJECT

public:
  ClientCameraVideoWidget(QWidget *parent);
  ~ClientCameraVideoWidget();
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
  void newFrame(const QImage &image);
  void newPixmapFrame(const QPixmap &pm);
};

#endif