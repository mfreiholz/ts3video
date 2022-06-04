#include "CameraTestWidget.h"
#include <QCamera>
#include <QBoxLayout>
#include <QVideoWidget>
#include <QCameraViewfinder>
#include <QComboBox>
#include <QDebug>
#include <QPainter>
#include <QScreen>
#include "libapp/elws.h"
#include <QVideoSurfaceFormat>

CameraTestWidget::CameraTestWidget(QWidget* parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	this->_ui.setupUi(this);
	QObject::connect(this->_ui.camInfo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CameraTestWidget::onCamInfoChanged);
	QObject::connect(this->_ui.pushButton, &QPushButton::clicked, this, &CameraTestWidget::onApplyClicked);

	// available cameras
	for (auto camInfo : QCameraInfo::availableCameras())
	{
		this->_ui.camInfo->addItem(camInfo.description(), camInfo.deviceName());
	}
}

CameraTestWidget::~CameraTestWidget() = default;

QCameraInfo CameraTestWidget::currentCameraInfo() const
{
	const auto deviceName = this->_ui.camInfo->currentData().toString();
	QCameraInfo camInfo;
	for (auto ci : QCameraInfo::availableCameras())
	{
		if (ci.deviceName() == deviceName)
		{
			camInfo = ci;
			break;
		}
	}
	return camInfo;
}

QSize CameraTestWidget::currentResolution() const
{
	return this->_ui.camResolution->currentData().toSize();
}

void CameraTestWidget::onCamInfoChanged(int index)
{
	// load resolutions
	auto camInfo = currentCameraInfo();
	QCamera cam(camInfo);
	cam.load();
	auto sizes = cam.supportedViewfinderResolutions();
	auto frameRateRanges = cam.supportedViewfinderFrameRateRanges();
	cam.unload();

	// resolutions
	this->_ui.camResolution->clear();
	for (auto s : sizes)
	{
		auto title = QString("%1x%2").arg(QString::number(s.width()), QString::number(s.height()));
		this->_ui.camResolution->addItem(title, s);
	}
}

void CameraTestWidget::onApplyClicked()
{
	// reset current used camera and viewfinder
	// ..

	// initialize new camera and viewfinder
	const auto camInfo = currentCameraInfo();
	const auto camResolution = currentResolution();

	auto camera = new QCamera(camInfo, this);
	camera->setCaptureMode(QCamera::CaptureMode::CaptureVideo);

	auto layout = new QBoxLayout(QBoxLayout::TopToBottom);

	if (false)
	{
		auto viewfinder = new QCameraViewfinder(this);
		camera->setViewfinder(viewfinder);
		layout->addWidget(viewfinder);
	}
	else
	{
		auto videoWidget = new MyVideoWidget(this);
		auto videoSurface = new MyVideoSurface(camInfo, this);
		camera->setViewfinder(videoSurface);
		QObject::connect(videoSurface, &MyVideoSurface::newImage, videoWidget, &MyVideoWidget::setImage);
		layout->addWidget(videoWidget);
	}

	auto sett = camera->viewfinderSettings();
	sett.setResolution(camResolution);
	camera->setViewfinderSettings(sett);

	camera->start();
	this->_ui.videoContainer->setLayout(layout);
}

/*********************************************************************/

MyVideoWidget::MyVideoWidget(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
}

MyVideoWidget::~MyVideoWidget()
{
}

void MyVideoWidget::setImage(const QImage& image)
{
	_currentImage = image;
	update();
}

void MyVideoWidget::paintEvent(QPaintEvent* e)
{
	if (_currentImage.isNull())
		return;

	QPainter p(this);
	if (_currentImage.isNull())
	{
		p.setPen(QColor(Qt::black));
		p.fillRect(rect(), Qt::SolidPattern);
	}
	else
	{
		p.drawImage(rect(), _currentImage);
	}
}

/*********************************************************************/

MyVideoSurface::MyVideoSurface(const QCameraInfo& camInfo, QObject* parent)
	: QAbstractVideoSurface(parent)
	, _camInfo(camInfo)
{
}

MyVideoSurface::~MyVideoSurface()
{
}

QList<QVideoFrame::PixelFormat> MyVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
	QList<QVideoFrame::PixelFormat> list;
	list << QVideoFrame::Format_RGB32 << QVideoFrame::Format_RGB24;
	return list;
}

bool MyVideoSurface::start(const QVideoSurfaceFormat& format)
{
	return QAbstractVideoSurface::start(format);
}

bool MyVideoSurface::present(const QVideoFrame& frame)
{
	QImage image = frame.image();
	if (image.isNull())
	{
		qDebug() << "Failed to image from frame";
		return false;
	}

	if (true)
	{
		image = image.transformed(QTransform().rotate(90));
	}

	// transform image orientation
	// @todo: do this in painter?
	if (true)
	{
		//auto screen = QGuiApplication::primaryScreen();
		//auto screenAngle = screen->angleBetween(screen->nativeOrientation(), screen->orientation());

		//int rotation;
		//if (_camInfo.position() == QCamera::BackFace) {
		//	rotation = (_camInfo.orientation() - screenAngle) % 360;
		//}
		//else {
		//	// Front position, compensate the mirror
		//	rotation = (360 - _camInfo.orientation() + screenAngle) % 360;
		//}
		//image = image.transformed(QTransform().rotate(rotation));
	}

	emit newImage(image);
	return true;
}
