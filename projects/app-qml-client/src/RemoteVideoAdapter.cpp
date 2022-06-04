#include "RemoteVideoAdapter.hpp"
#include "Logging.hpp"
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoSurfaceFormat>

RemoteVideoAdapter::RemoteVideoAdapter(QObject* parent)
	: QObject(parent)
{}

RemoteVideoAdapter::~RemoteVideoAdapter() = default;

QAbstractVideoSurface* RemoteVideoAdapter::getVideoSurface() const
{
	return m_videoSurface;
}

void RemoteVideoAdapter::setVideoSurface(QAbstractVideoSurface* surface)
{
	qCDebug(logCore, "Enter RemoteVideoAdapter::setVideoSurface(0x%x)", surface);
	if (surface)
	{
		qCDebug(logCore, "IsActive=%d", surface->isActive() ? 1 : 0);
		if (!surface->isActive())
		{
			QVideoSurfaceFormat fmt(QSize(1920, 1080), QVideoFrame::Format_YV12);
			//fmt.setScanLineDirection(QVideoSurfaceFormat::TopToBottom);
			//fmt.setYCbCrColorSpace(QVideoSurfaceFormat::YCbCr_xvYCC709);
			//QVideoSurfaceFormat fmt(QSize(1920, 1080), QVideoFrame::Format_RGB24);
			const auto nfmt = surface->nearestFormat(fmt);
			if (!surface->start(nfmt))
			{
				qCCritical(logCore, "Can't start video surface: %d", surface->error());
			}
			else
			{
				qCDebug(logCore, "Surface started");
			}
		}
	}
	m_videoSurface = surface;
	emit videoSurfaceChanged();
	qCDebug(logCore, "Leave RemoteVideoAdapter::setVideoSurface(0x%x)", surface);
}

int RemoteVideoAdapter::getClientId() const
{
	return m_clientId;
}

void RemoteVideoAdapter::setClientId(int clientId)
{
	qCDebug(logCore, "Enter RemoteVideoAdapter::setClientId(%d)", clientId);
	m_clientId = clientId;
	emit clientIdChanged();
	qCDebug(logCore, "Leave RemoteVideoAdapter::setClientId(%d)", clientId);
}

void RemoteVideoAdapter::setVideoFrame(YuvFrameRefPtr frame)
{
	qCDebug(logCore, "Enter RemoteVideoAdapter::setVideoFrame(frame)");
	if (m_videoSurface && m_videoSurface->isActive())
	{
		auto image = frame->toQImage();
		QVideoFrame f(image);
		if (!m_videoSurface->present(f))
		{
			qCCritical(logCore, "Can't present frame: %d", m_videoSurface->error());
		}

		//if (!m_videoSurface->isActive())
		//{
		//	//auto size = QSize(frame->width, frame->height);
		//	//auto pixelFormat = QVideoFrame::Format_RGB24;
		//	//QVideoSurfaceFormat fmt(size, pixelFormat);
		//	//m_videoSurface->start(fmt);
		//}

		//const auto img = frame->toQImage();
		//QVideoFrame f(img);
		//if (!m_videoSurface->present(f))
		//{
		//	qCDebug(logCore, "Can't present frame");
		//}
	}
	qCDebug(logCore, "Leave RemoteVideoAdapter::setVideoFrame(frame)");
}
