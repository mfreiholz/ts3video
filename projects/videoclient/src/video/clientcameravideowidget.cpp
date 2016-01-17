#include "clientcameravideowidget.h"

#include <QTime>
#include <QBoxLayout>
#include <QLabel>
#include <QCameraInfo>
#include <QMessageBox>

#include "humblelogging/api.h"

#include "elws.h"

#include "networkclient/networkclient.h"
#include "video/conferencevideowindow.h"
#include "videowidget.h"

HUMBLE_LOGGER(HL, "client.camera");

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(ConferenceVideoWindow* window, QWidget* parent) :
	QWidget(parent),
	_window(window),
	_nc(window->networkClient()),
	_camera(window->camera()),
	_videoWidget(nullptr)
{
	// Load camera and forward frames to grabber.
	_grabber.reset(new CameraFrameGrabber(_window->options().cameraResolution, this));
	_camera->setViewfinder(_grabber.data());

	// GUI
	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	// Create widget to render video frames.
	if (_window->options().uiVideoHardwareAccelerationEnabled)
		_videoWidget = new VideoWidget(VideoWidget::OpenGL_ImageWidget);
	else
		_videoWidget = new VideoWidget(VideoWidget::CPU);
	mainLayout->addWidget(_videoWidget, 1);

	// EVENTS
	// Grabber events.
	QObject::connect(_grabber.data(), &CameraFrameGrabber::newQImage, this, &ClientCameraVideoWidget::onNewQImage);

	// Camera events.
	QObject::connect(_camera.data(), static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error), [this](QCamera::Error error)
	{
		HL_ERROR(HL, QString("Camera error (error=%1; message=%2)").arg(error).arg(_camera->errorString()).toStdString());
		QMessageBox::critical(this, QString(), tr("There is a problem with the camera:\n\nCode: %1\nMessage: %2\n\nA restart of the application might fix this problem.").arg(error).arg(_camera->errorString()));
	});
	QObject::connect(_camera.data(), &QCamera::lockFailed, [this]()
	{
		HL_ERROR(HL, QString("Camera lock failed").toStdString());
		QMessageBox::critical(this, QString(), tr("Can not lock camera. It's seems to be in use by another process."));
	});
}

ClientCameraVideoWidget::~ClientCameraVideoWidget()
{
	if (_camera)
	{
		_camera->setViewfinder((QAbstractVideoSurface*)nullptr);
	}
	if (_grabber)
	{
		_grabber->disconnect(this);
	}
}

QSharedPointer<NetworkClient> ClientCameraVideoWidget::networkClient() const
{
	return _nc;
}

QSharedPointer<QCamera> ClientCameraVideoWidget::camera() const
{
	return _camera;
}

void ClientCameraVideoWidget::setFrame(const QImage& f)
{
	_videoWidget->setFrame(f);
}

void ClientCameraVideoWidget::onNewQImage(const QImage& image)
{
	_videoWidget->setFrame(image);
	_nc->sendVideoFrame(image);
}