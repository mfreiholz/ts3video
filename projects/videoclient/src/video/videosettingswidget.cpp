#include "videosettingswidget.h"

#include <QtMultimedia/QCameraInfo>

VideoSettingsDialog::VideoSettingsDialog(QWidget* parent) :
	QDialog(parent)
{
	_ui.setupUi(this);

	// Devices
	auto infos = QCameraInfo::availableCameras();
	for (auto i = 0; i < infos.size(); ++i)
	{
		_ui.devices->addItem(QIcon(), infos[i].description(), infos[i].deviceName());
	}

	// Resolutions
	QList<QSize> dims;

	// 4:3
	dims.append(QSize(640, 480));
	dims.append(QSize(800, 600));
	dims.append(QSize(1024, 768));
	dims.append(QSize(1280, 960));
	dims.append(QSize(1600, 1200));

	// 16:9
	dims.append(QSize(852, 480));
	dims.append(QSize(1280, 720));
	dims.append(QSize(1365, 768));
	dims.append(QSize(1600, 900));
	dims.append(QSize(1920, 1080));

	// 16:10
	dims.append(QSize(1440, 900));
	dims.append(QSize(1680, 1050));
	dims.append(QSize(1920, 1200));
	dims.append(QSize(2560, 1600));

	for (auto i = 0; i < dims.size(); ++i)
	{
		_ui.resolutions->addItem(QIcon(), QString("%1x%2").arg(dims[i].width()).arg(dims[i].height()), dims[i]);
	}

	// Quality / Bandwidth / Bitrate
	// 1 = 0,125 KB/s
	// 1024 = 128 KB/s
	_ui.quality->setMinimum(50);
	_ui.quality->setMaximum(1024);
	_ui.quality->setValue(_ui.quality->minimum());

	_ui.qualityValue->setMinimum(_ui.quality->minimum());
	_ui.qualityValue->setMaximum(_ui.quality->maximum());
	_ui.qualityValue->setValue(_ui.quality->value());

	// Button events
	QObject::connect(_ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(_ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void VideoSettingsDialog::preselect(const ConferenceVideoWindow::Options& opts)
{
	_opts = opts;

	// Device
	auto index = _ui.devices->findData(opts.cameraDeviceId);
	if (index >= 0)
	{
		_ui.devices->setCurrentIndex(index);
	}
	_ui.devices->addItem(tr("No camera (Viewer mode)"));

	// Resolution
	index = _ui.resolutions->findData(opts.cameraResolution);
	if (index >= 0)
	{
		_ui.devices->setCurrentIndex(index);
	}

	// Quality
	_ui.quality->setValue(opts.cameraBitrate);
}

const ConferenceVideoWindow::Options& VideoSettingsDialog::values()
{
	_opts.cameraDeviceId = _ui.devices->currentData().toString();
	_opts.cameraResolution = _ui.resolutions->currentData().toSize();
	_opts.cameraBitrate = _ui.quality->value();
	return _opts;
}