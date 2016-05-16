#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QImage>
#include <QSize>
#include <QRect>
#include <QPoint>
#include <QVideoFrame>
#include <QAbstractVideoSurface>
class QWidget;

class CameraFrameGrabber : public QAbstractVideoSurface
{
	Q_OBJECT

public:
	CameraFrameGrabber(const QSize& resolution, QWidget* widget, QObject* parent);
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
	bool present(const QVideoFrame& frame);

signals:
	void newQImage(const QImage& image);

public:
	bool _firstFrame;
	QSize _targetSize;

	QRect _imageRect;
	QPoint _imageOffset;

	QWidget* _widget;

public:
	QVideoFrame current;
};

#endif