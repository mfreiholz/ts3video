#include "clientcameravideowidget.h"

#include <QTime>
#include <QBoxLayout>
#include <QLabel>
#include <QCameraInfo>
#include <QCamera>
#include <QMessageBox>

#include "humblelogging/api.h"

#include "elws.h"

#include "networkclient/networkclient.h"
#include "cameraframegrabber.h"
#include "videowidget.h"

HUMBLE_LOGGER(HL, "client.camera");

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(const QSharedPointer<NetworkClient>& ts3vc, const QSharedPointer<QCamera>& camera, QWidget* parent) :
	QWidget(parent),
	_ts3vc(ts3vc),
	_camera(camera),
	_videoWidget(nullptr)
{
	// Load camera and forward frames to grabber.
	auto grabber = new CameraFrameGrabber(this);
	camera->setViewfinder(grabber);
	//camera->start();

	// GUI
	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	// Create widget to render video frames.
	auto videoWidget = new VideoWidget(VideoWidget::OpenGL_ImageWidget);
	mainLayout->addWidget(videoWidget, 1);
	_videoWidget = videoWidget;

	// EVENTS
	// Grabber events.
	QObject::connect(grabber, &CameraFrameGrabber::newQImage, [this](const QImage & image)
	{
		_videoWidget->setFrame(image);
	});
	QObject::connect(grabber, &CameraFrameGrabber::newQImage, [this](const QImage & image)
	{
		if (_ts3vc->isReadyForStreaming())
		{
			_ts3vc->sendVideoFrame(image);
		}
	});
	// Camera events.
	QObject::connect(camera.data(), static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error), [this, camera](QCamera::Error error)
	{
		HL_ERROR(HL, QString("Camera error (error=%1; message=%2)").arg(error).arg(camera->errorString()).toStdString());
		QMessageBox::critical(this, QString(), tr("There is a problem with the camera:\n\nCode: %1\nMessage: %2\n\nA restart of the application might fix this problem.").arg(error).arg(camera->errorString()));
	});
	QObject::connect(camera.data(), &QCamera::lockFailed, [this]()
	{
		HL_ERROR(HL, QString("Camera lock failed").toStdString());
		QMessageBox::critical(this, QString(), tr("Can not lock camera. It's seems to be in use by another process."));
	});
}

ClientCameraVideoWidget::~ClientCameraVideoWidget()
{
}

QSharedPointer<NetworkClient> ClientCameraVideoWidget::networkClient() const
{
	return _ts3vc;
}

QSharedPointer<QCamera> ClientCameraVideoWidget::camera() const
{
	return _camera;
}

void ClientCameraVideoWidget::setFrame(const QImage& f)
{
	_videoWidget->setFrame(f);
}