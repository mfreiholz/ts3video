#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QPointer>
#include <QWidget>
#include <QCamera>
class QImage;
class QCameraInfo;
class NetworkClient;
class VideoWidget;

class ClientCameraVideoWidget : public QWidget
{
	Q_OBJECT

public:
	ClientCameraVideoWidget(const QSharedPointer<NetworkClient>& nc, const QSharedPointer<QCamera>& camera, QWidget* parent);
	~ClientCameraVideoWidget();
	QSharedPointer<NetworkClient> networkClient() const;
	QSharedPointer<QCamera> camera() const;

public slots:
	void setFrame(const QImage& f);

private:
	QSharedPointer<NetworkClient> _ts3vc;
	QSharedPointer<QCamera> _camera;
	VideoWidget* _videoWidget;
};

#endif