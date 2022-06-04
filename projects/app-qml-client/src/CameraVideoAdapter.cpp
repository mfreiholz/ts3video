#include "CameraVideoAdapter.hpp"
#include "Logging.hpp"
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QCamera>

CameraVideoAdapter::CameraVideoAdapter(QObject* parent)
	: QObject(parent)
	, m_camera(nullptr)
	, m_cameraSurface(nullptr)
	, m_videoSurface(nullptr)
{}

CameraVideoAdapter::~CameraVideoAdapter() = default;

QAbstractVideoSurface* CameraVideoAdapter::getVideoSurface() const
{
	return m_videoSurface;
}

void CameraVideoAdapter::setVideoSurface(QAbstractVideoSurface* surface)
{
	qCDebug(logCore, "Enter CameraVideoAdapter::setVideoSurface(0x%x)", surface);
	// Reset current one.
	if (m_cameraSurface)
	{
		m_cameraSurface->setTargetVideoSurface(nullptr);
		m_cameraSurface.reset();
	}
	// Setup new surface.
	m_videoSurface = surface;
	if (m_videoSurface)
	{
		m_cameraSurface = std::make_unique<CameraVideoSurface>(nullptr);
		m_cameraSurface->setTargetVideoSurface(m_videoSurface);
		QObject::connect(m_cameraSurface.get(), &CameraVideoSurface::firstFrame, this, &CameraVideoAdapter::firstFrame);
		QObject::connect(m_cameraSurface.get(), &CameraVideoSurface::newCameraImage, this, &CameraVideoAdapter::newCameraImage);
	}
	emit videoSurfaceChanged();
	qCDebug(logCore, "Leave CameraVideoAdapter::setVideoSurface(0x%x)", surface);
}

QString CameraVideoAdapter::getDeviceName() const
{
	return m_deviceName;
}

void CameraVideoAdapter::setDeviceName(const QString& deviceName)
{
	qCDebug(logCore, "Call CameraVideoAdapter::setDeviceName(%s)", deviceName);
	if (m_deviceName == deviceName)
	{
		qCDebug(logCore, "Tried to set same device name. Do nothing.");
		return;
	}
	// Reset current used QCamera.
	if (m_camera)
	{
		m_camera->setViewfinder(static_cast<QAbstractVideoSurface*>(nullptr));
		m_camera->stop();
		m_camera.reset();
	}
	// Setup QCamera based on new device name.
	m_deviceName = deviceName;
	m_camera = std::make_unique<QCamera>(m_deviceName.toLatin1(), nullptr);
	m_camera->setViewfinder(m_cameraSurface.get());
	if (m_cameraEnabled)
		m_camera->start();
	emit deviceNameChanged();
}

bool CameraVideoAdapter::isCameraEnabled() const
{
	return m_cameraEnabled;
}

void CameraVideoAdapter::setCameraEnabled(bool onoff)
{
	const auto changed = m_cameraEnabled != onoff;
	if (!changed)
		return;

	m_cameraEnabled = onoff;
	if (m_camera)
	{
		if (m_cameraEnabled)
			m_camera->start();
		else
			m_camera->stop();
	}
	emit cameraEnabledChanged();
}
