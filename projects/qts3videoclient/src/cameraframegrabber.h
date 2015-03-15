#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QImage>
#include <QSize>
#include <QRect>
#include <QPoint>
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