#include "cameraframegrabber.h"

#include "humblelogging/api.h"

#include "libapp/elws.h"

HUMBLE_LOGGER(HL, "client.camera");

///////////////////////////////////////////////////////////////////////

CameraFrameGrabber::CameraFrameGrabber(const QSize& resolution, QObject* parent) :
	QAbstractVideoSurface(parent),
	_firstFrame(true),
	_targetSize(resolution)//(IFVS_CLIENT_VIDEO_SIZE)
{
	setNativeResolution(_targetSize);

	_pixelFormats
		<< QVideoFrame::Format_RGB32
		<< QVideoFrame::Format_BGR32
		<< QVideoFrame::Format_ARGB32
		<< QVideoFrame::Format_RGB24
		<< QVideoFrame::Format_BGR24
		<< QVideoFrame::Format_RGB565
		<< QVideoFrame::Format_AYUV444
		<< QVideoFrame::Format_YUV444
		<< QVideoFrame::Format_YV12
		<< QVideoFrame::Format_YUV420P;
}

QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
	switch (handleType)
	{
	case QAbstractVideoBuffer::HandleType::NoHandle:
		return _pixelFormats;
	}
	return QList<QVideoFrame::PixelFormat>();
}

bool CameraFrameGrabber::present(const QVideoFrame& frame)
{
	if (!frame.isValid())
	{
		return false;
	}

	QVideoFrame f(frame);
	auto imageFormat = QVideoFrame::imageFormatFromPixelFormat(f.pixelFormat());
	if (imageFormat == QImage::Format_Invalid)
	{
		HL_ERROR(HL, QString("Invalid image format for video frame.").toStdString());
		return false;
	}

	if (_firstFrame)
	{
		HL_INFO(HL, QString("Camera first frame (format=%1; width=%2; height=%3)").arg(f.pixelFormat()).arg(f.width()).arg(f.height()).toStdString());
		_firstFrame = false;

		// Calculate target rect for centered-scaling.
		auto surfaceRect = QRect(QPoint(0, 0), _targetSize);
		_imageRect = QRect(QPoint(0, 0), f.size());
		ELWS::calcScaledAndCenterizedImageRect(surfaceRect, _imageRect, _imageOffset);
	}

	if (f.map(QAbstractVideoBuffer::ReadOnly))
	{
		// Create copy via copy() or mirrored(). At least we need a copy as long as we don't directly print here.
		auto image = QImage(f.bits(), f.width(), f.height(), imageFormat);
#ifdef _WIN32
		image = image.mirrored();
#endif
		if (image.size() != _imageRect.size())
		{
			image = image.scaled(_imageRect.size());
		}
		if (_imageOffset.x() != 0)
		{
			image = image.copy(_imageOffset.x(), 0, _targetSize.width(), _targetSize.height());
		}
		else if (_imageOffset.y() != 0)
		{
			image = image.copy(0, _imageOffset.y(), _targetSize.width(), _targetSize.height());
		}
#ifndef _WIN32
		else
		{
			image = image.copy();
		}
#endif
		emit newQImage(image);
		f.unmap();
	}
	return true;
}
