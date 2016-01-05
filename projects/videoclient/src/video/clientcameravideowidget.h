#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QPointer>
#include <QWidget>
#include <QCamera>
#include <QScopedPointer>
#include "video/cameraframegrabber.h"
class QImage;
class QCameraInfo;
class NetworkClient;
class VideoWidget;
class ConferenceVideoWindow;

class ClientCameraVideoWidget : public QWidget
{
	Q_OBJECT

public:
	ClientCameraVideoWidget(ConferenceVideoWindow* window, QWidget* parent);
	virtual ~ClientCameraVideoWidget();
	QSharedPointer<NetworkClient> networkClient() const;
	QSharedPointer<QCamera> camera() const;

public slots:
	void setFrame(const QImage& f);

private slots:
	void onNewQImage(const QImage& image);

private:
	ConferenceVideoWindow* _window;
	QSharedPointer<NetworkClient> _nc;
	QSharedPointer<QCamera> _camera;
	QScopedPointer<CameraFrameGrabber> _grabber;
	VideoWidget* _videoWidget;
};

#endif