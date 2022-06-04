#include "CameraVideoSurface.hpp"
#include "Logging.hpp"

CameraVideoSurface::CameraVideoSurface(QObject* parent)
	: QAbstractVideoSurface(parent)
	, m_targetSurface(nullptr)
{}

CameraVideoSurface::~CameraVideoSurface() = default;

void CameraVideoSurface::setTargetVideoSurface(QAbstractVideoSurface* surface)
{
	m_targetSurface = surface;
}

bool CameraVideoSurface::isFormatSupported(const QVideoSurfaceFormat& format) const
{
	return m_targetSurface->isFormatSupported(format);
}

QVideoSurfaceFormat CameraVideoSurface::nearestFormat(const QVideoSurfaceFormat& format) const
{
	return m_targetSurface->nearestFormat(format);
}

QList<QVideoFrame::PixelFormat> CameraVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
	if (!m_targetSurface)
		return {};
	const auto formats = m_targetSurface->supportedPixelFormats(type);
	return formats;
}

bool CameraVideoSurface::present(const QVideoFrame& f)
{
	if (!f.isValid())
		return false;

	if (!m_firstFrame)
	{
		m_firstFrame = std::make_optional<FirstFrameInfo>();
		m_firstFrame->imageFormat = QVideoFrame::imageFormatFromPixelFormat(f.pixelFormat());
		qCDebug(logCore) << QString("First frame: (pixelformat=%1; imageFormat=%2 width=%3; height=%4)")
								.arg(f.pixelFormat())
								.arg(m_firstFrame->imageFormat)
								.arg(f.width())
								.arg(f.height());
		emit firstFrame(f.width(), f.height());
		return true; // Ignore first frame - TESTING
	}
	// Encode for video.
	QVideoFrame frame(f);
	if (frame.map(QAbstractVideoBuffer::ReadOnly))
	{
		QImage image = QImage(frame.bits(), frame.width(), frame.height(), m_firstFrame->imageFormat);

		// @todo Here we do a hard copy of image data.
		// this is required that the bytes don't get invalid in later encoding thread.
		// Alternatively we could directly encode here in the same thread?
		image = image.mirrored();

		emit newCameraImage(image);
	}
	// Present to QML.
	const auto presented = m_targetSurface->present(f);
	frame.unmap();
	return presented;
}

bool CameraVideoSurface::start(const QVideoSurfaceFormat& format)
{
	m_firstFrame = std::nullopt;
	return m_targetSurface->start(format);
}

void CameraVideoSurface::stop()
{
	m_targetSurface->stop();
}
