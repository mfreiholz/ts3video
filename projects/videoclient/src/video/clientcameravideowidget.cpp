#include "clientcameravideowidget.h"

#include <QTime>
#include <QBoxLayout>
#include <QLabel>
#include <QCameraInfo>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>

#include "humblelogging/api.h"

#include "elws.h"

#include "networkclient/networkclient.h"
#include "video/conferencevideowindow.h"
#include "videowidget.h"

HUMBLE_LOGGER(HL, "client.camera");

static QTransform flipHorizontal(const QRect& surfaceRect, const QTransform& t)
{
	QTransform transform(t);
	qreal m11 = transform.m11();    // Horizontal scaling
	qreal m12 = transform.m12();    // Vertical shearing
	qreal m13 = transform.m13();    // Horizontal Projection
	qreal m21 = transform.m21();    // Horizontal shearing
	qreal m22 = transform.m22();    // vertical scaling
	qreal m23 = transform.m23();    // Vertical Projection
	qreal m31 = transform.m31();    // Horizontal Position (DX)
	qreal m32 = transform.m32();    // Vertical Position (DY)
	qreal m33 = transform.m33();    // Addtional Projection Factor

	qreal scale = m11;
	m11 = -m11;

	if (m31 > 0)
		m31 = 0;
	else
		m31 = (surfaceRect.width() * scale);

	transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
	return std::move(transform);
}

static QTransform flipVertical(const QRect& surfaceRect, const QTransform& t)
{
	QTransform transform(t);
	qreal m11 = transform.m11();    // Horizontal scaling
	qreal m12 = transform.m12();    // Vertical shearing
	qreal m13 = transform.m13();    // Horizontal Projection
	qreal m21 = transform.m21();    // Horizontal shearing
	qreal m22 = transform.m22();    // vertical scaling
	qreal m23 = transform.m23();    // Vertical Projection
	qreal m31 = transform.m31();    // Horizontal Position (DX)
	qreal m32 = transform.m32();    // Vertical Position (DY)
	qreal m33 = transform.m33();    // Addtional Projection Factor

	// We need this in a minute
	qreal scale = m22;

	// Vertical flip
	m22 = -m22;

	// Re-position back to origin
	if (m32 > 0)
		m32 = 0;
	else
		m32 = (surfaceRect.height() * scale);

	// Write back to the matrix
	transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
	return std::move(transform);
}

///////////////////////////////////////////////////////////////////////

ClientCameraVideoWidget::ClientCameraVideoWidget(ConferenceVideoWindow* window, QWidget* parent) :
	QWidget(parent),
	_window(window),
	_nc(window->networkClient()),
	_camera(window->camera()),
	_videoWidget(nullptr)
{
	setAutoFillBackground(true);

	// Load camera and forward frames to grabber.
	_grabber.reset(new CameraFrameGrabber(_window->options().cameraResolution, nullptr, this));
	_camera->setViewfinder(_grabber.data());

	// GUI
	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	// Create widget to render video frames.
	if (false && _window->options().uiVideoHardwareAccelerationEnabled)
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
	if (_videoWidget)
		_videoWidget->setFrame(f);
}

void ClientCameraVideoWidget::paintEvent(QPaintEvent* e)
{
	auto f = _grabber->current;
	if (f.isValid())
	{
		if (f.map(QAbstractVideoBuffer::ReadOnly))
		{
			const auto imageFormat = QVideoFrame::imageFormatFromPixelFormat(f.pixelFormat());
			const auto image = QImage(f.bits(), f.width(), f.height(), imageFormat);

			QPainter p(this);

			const auto surfaceRect = rect();
			const auto imageRect = image.rect();
			auto surfaceRatio = (float)surfaceRect.width() / (float)surfaceRect.height();
			auto imageRatio = (float)imageRect.width() / (float)imageRect.height();
			auto scaleFactor = 1.0F;
			if (surfaceRatio < imageRatio)
				scaleFactor = (float)surfaceRect.height() / (float)imageRect.height();
			else
				scaleFactor = (float)surfaceRect.width() / (float)imageRect.width();

			p.setTransform(flipVertical(surfaceRect, p.transform()));
			p.scale(scaleFactor, scaleFactor);
			p.drawImage(0, 0, image);
			f.unmap();
		}
	}
}

void ClientCameraVideoWidget::onNewQImage(const QImage& image)
{
	if (_videoWidget)
		_videoWidget->setFrame(image);
	_nc->sendVideoFrame(image);
}