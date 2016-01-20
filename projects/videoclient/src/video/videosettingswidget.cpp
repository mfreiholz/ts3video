#include "videosettingswidget.h"

#include <QtMultimedia/QCameraInfo>

#include "qtasync.h"

#include "videolib/src/virtualserverconfigentity.h"

///////////////////////////////////////////////////////////////////////

#include <QAbstractListModel>
class ResolutionListModel : public QAbstractListModel
{
public:
	ResolutionListModel(ConferenceVideoWindow* window, const QCameraInfo& cameraInfo, QObject* parent) : QAbstractListModel(parent), _window(window)
	{
		const auto& serverConfig = window->networkClient()->serverConfig();
		QCamera cam(cameraInfo);
		cam.load();
		_resolutions = cam.supportedViewfinderResolutions();
	}

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return _resolutions.size();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		const auto& serverConfig = _window->networkClient()->serverConfig();
		const auto& size = _resolutions[index.row()];
		if (!serverConfig.isResolutionSupported(size))
			return QAbstractListModel::flags(index) ^ Qt::ItemIsEnabled;
		return QAbstractListModel::flags(index);
	}

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
	{
		if (index.row() >= _resolutions.size())
			return QVariant();

		const auto& serverConfig = _window->networkClient()->serverConfig();
		const auto& size = _resolutions[index.row()];

		switch (role)
		{
		case Qt::DisplayRole:
		{
			if (!serverConfig.isResolutionSupported(size))
				return QString("%1x%2 (Not supported by server)").arg(size.width()).arg(size.height());
			else
				return QString("%1x%2").arg(size.width()).arg(size.height());
			break;
		}
		case Qt::UserRole:
		{
			return size;
		}
		}
		return QVariant();
	}

private:
	ConferenceVideoWindow* _window;
	QList<QSize> _resolutions;
};

///////////////////////////////////////////////////////////////////////

VideoSettingsDialog::VideoSettingsDialog(ConferenceVideoWindow* window, QWidget* parent) :
	QDialog(parent),
	_window(window)
{
	_ui.setupUi(this);

	// Devices
	const auto infos = QCameraInfo::availableCameras();
	for (auto i = 0; i < infos.size(); ++i)
	{
		_ui.devices->addItem(QIcon(), infos[i].description(), infos[i].deviceName());
	}
	_ui.devices->addItem(QIcon(), tr("No camera (Viewer mode)"), QVariant(QString()));

	// Quality / Bandwidth / Bitrate
	// 1 = 0,125 KByte/s
	// 1024 = 128 KByte/s
	_ui.quality->setMinimum(50);
	_ui.quality->setMaximum(1024);
	_ui.quality->setValue(_ui.quality->minimum());

	_ui.qualityValue->setMinimum(_ui.quality->minimum());
	_ui.qualityValue->setMaximum(_ui.quality->maximum());
	_ui.qualityValue->setValue(_ui.quality->value());

	// Ui events
	QObject::connect(_ui.quality, &QSlider::valueChanged, _ui.qualityValue, &QSpinBox::setValue);
	QObject::connect(_ui.devices, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &VideoSettingsDialog::onCurrentDeviceIndexChanged);
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
		const bool forceUpdate = _ui.devices->currentIndex() == index;
		_ui.devices->setCurrentIndex(index);
		if (forceUpdate)
			onCurrentDeviceIndexChanged(index);
	}

	// Resolution
	index = _ui.resolutions->findData(opts.cameraResolution);
	if (index >= 0)
	{
		_ui.resolutions->setCurrentIndex(index);
	}

	// Quality
	_ui.quality->setValue(opts.cameraBitrate);

	// Hardware acceleration
	_ui.hardwareAcceleration->setChecked(opts.uiVideoHardwareAccelerationEnabled);

	// Auto enable
	_ui.autoEnable->setChecked(opts.cameraAutoEnable);
}

const ConferenceVideoWindow::Options& VideoSettingsDialog::values()
{
	_opts.cameraDeviceId = _ui.devices->currentData().toString();
	_opts.cameraResolution = _ui.resolutions->currentData().toSize();
	_opts.cameraBitrate = _ui.quality->value();
	_opts.uiVideoHardwareAccelerationEnabled = _ui.hardwareAcceleration->isChecked();
	_opts.cameraAutoEnable = _ui.autoEnable->isChecked();
	return _opts;
}

void VideoSettingsDialog::onCurrentDeviceIndexChanged(int index)
{
	const auto& serverConfig = _window->networkClient()->serverConfig();

	const auto deviceId = _ui.devices->currentData().toString();
	_ui.resolutions->setEnabled(!deviceId.isEmpty());
	_ui.quality->setEnabled(!deviceId.isEmpty());
	_ui.qualityValue->setEnabled(!deviceId.isEmpty());
	_ui.autoEnable->setEnabled(!deviceId.isEmpty());

	// Detect supported resolutions and fill combo box
	_ui.resolutions->clear();
	if (!deviceId.isEmpty())
	{
		QCameraInfo cameraInfo;
		auto infos = QCameraInfo::availableCameras();
		foreach (const auto& info, infos)
		{
			if (info.deviceName() == deviceId)
			{
				cameraInfo = info;
				break;
			}
		}
		delete _ui.resolutions->model();
		_ui.resolutions->setModel(new ResolutionListModel(_window, cameraInfo, this));
	}
}