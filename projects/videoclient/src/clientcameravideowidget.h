#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QPointer>
#include <QWidget>
#include <QCamera>
#include <QScopedPointer>
#include "cameraframegrabber.h"
class QImage;
class QCameraInfo;
class NetworkClient;
class VideoWidget;

class ClientCameraVideoWidget : public QWidget
{
	Q_OBJECT

public:
	ClientCameraVideoWidget(const QSharedPointer<NetworkClient>& nc, const QSharedPointer<QCamera>& camera, QWidget* parent);
	virtual ~ClientCameraVideoWidget();
	QSharedPointer<NetworkClient> networkClient() const;
	QSharedPointer<QCamera> camera() const;

public slots:
	void setFrame(const QImage& f);

private slots:
	void onNewQImage(const QImage& image);

private:
	QSharedPointer<NetworkClient> _ts3vc;
	QSharedPointer<QCamera> _camera;
	QScopedPointer<CameraFrameGrabber> _grabber;
	VideoWidget* _videoWidget;
};

#endif