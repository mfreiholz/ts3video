#pragma once
#include <QImage>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <optional>

/*
	Acts as a proxy.
	Grabs frames from QCamera and calls  "present" of another surface.
*/
class CameraVideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT

public:
	explicit CameraVideoSurface(QObject* parent);
	~CameraVideoSurface() override;

	void setTargetVideoSurface(QAbstractVideoSurface* surface);
	bool isFormatSupported(const QVideoSurfaceFormat& format) const override;
	QVideoSurfaceFormat nearestFormat(const QVideoSurfaceFormat& format) const override;
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
	bool present(const QVideoFrame& frame) override;
	bool start(const QVideoSurfaceFormat& format) override;
	void stop() override;

signals:
	void firstFrame(int width, int height);
	void newCameraImage(const QImage& image);

private:
	struct FirstFrameInfo
	{
		QImage::Format imageFormat;
	};

	QAbstractVideoSurface* m_targetSurface;
	std::optional<FirstFrameInfo> m_firstFrame;
};
