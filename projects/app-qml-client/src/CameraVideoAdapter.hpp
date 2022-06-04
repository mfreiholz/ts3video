#pragma once
#include "CameraVideoSurface.hpp"
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <memory>
class QCamera;
class QAbstractVideoSurface;

class CameraVideoAdapter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QAbstractVideoSurface* videoSurface READ getVideoSurface WRITE setVideoSurface NOTIFY videoSurfaceChanged)
	Q_PROPERTY(QString deviceName READ getDeviceName WRITE setDeviceName NOTIFY deviceNameChanged)
	Q_PROPERTY(bool cameraEnabled READ isCameraEnabled WRITE setCameraEnabled NOTIFY cameraEnabledChanged)

public:
	explicit CameraVideoAdapter(QObject* parent);
	CameraVideoAdapter(const CameraVideoAdapter&) = delete;
	CameraVideoAdapter& operator=(const CameraVideoAdapter&) = delete;
	~CameraVideoAdapter() override;

	QAbstractVideoSurface* getVideoSurface() const;
	void setVideoSurface(QAbstractVideoSurface* surface);

	QString getDeviceName() const;
	void setDeviceName(const QString& deviceName);

	bool isCameraEnabled() const;
	void setCameraEnabled(bool onoff);

signals:
	void videoSurfaceChanged();
	void deviceNameChanged();
	void cameraEnabledChanged();
	void firstFrame(int width, int height);
	void newCameraImage(const QImage& image);

private:
	QPointer<QAbstractVideoSurface> m_videoSurface; //< Pointer to the video surface provided by QML.
	std::unique_ptr<CameraVideoSurface> m_cameraSurface; //<

	// Indicates the setting of the user.
	QString m_deviceName;
	bool m_cameraEnabled = false;
	std::unique_ptr<QCamera> m_camera;
};
