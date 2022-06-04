#pragma once
#include "libapp/yuvframe.h"
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtQml/QQmlEngine>

class RemoteVideoAdapter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QAbstractVideoSurface* videoSurface READ getVideoSurface WRITE setVideoSurface NOTIFY videoSurfaceChanged)
	Q_PROPERTY(int clientId READ getClientId WRITE setClientId NOTIFY clientIdChanged)

public:
	explicit RemoteVideoAdapter(QObject* parent = nullptr);
	RemoteVideoAdapter(const RemoteVideoAdapter&) = delete;
	RemoteVideoAdapter& operator=(const RemoteVideoAdapter&) = delete;
	~RemoteVideoAdapter() override;

	QAbstractVideoSurface* getVideoSurface() const;
	void setVideoSurface(QAbstractVideoSurface* surface);

	int getClientId() const;
	void setClientId(int clientId);

	void setVideoFrame(YuvFrameRefPtr frame);

	static void registerQmlTypes()
	{
		qmlRegisterType<RemoteVideoAdapter>("RemoteVideoAdapter", 1, 0, "RemoteVideoAdapter");
	}

signals:
	void videoSurfaceChanged();
	void clientIdChanged();

private:
	QPointer<QAbstractVideoSurface> m_videoSurface; //< Pointer to the video surface provided by QML.
	int m_clientId = -1;
};
