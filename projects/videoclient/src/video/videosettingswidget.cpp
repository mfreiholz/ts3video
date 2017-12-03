#include "videosettingswidget.h"

#include <QList>
#include <QPair>
#include <QtMultimedia/QCameraInfo>

#include "libqtasync/qtasync.h"

#include "videolib/virtualserverconfigentity.h"

#include "util/qwidgetutil.h"

///////////////////////////////////////////////////////////////////////

#include <QAbstractListModel>
class ResolutionListModel : public QAbstractListModel
{
public:
	ResolutionListModel(const QSharedPointer<NetworkClient>& nc,
						const QCameraInfo& cameraInfo, QObject* parent) :
		QAbstractListModel(parent),
		_nc(nc)
	{
		const auto& serverConfig = _nc->serverConfig();
		QCamera cam(cameraInfo);
		cam.load();
//#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0) && _WIN32
//		_resolutions = cam.supportedViewfinderResolutions();
//#else
		_resolutions = QList<QSize>()
					   << QSize(160, 120)
					   << QSize(176, 144)
					   << QSize(320, 176)
					   << QSize(320, 240)
					   << QSize(352, 288)
					   << QSize(432, 240)
					   << QSize(544, 288)
					   << QSize(640, 360)
					   << QSize(640, 480)
					   << QSize(752, 416)
					   << QSize(800, 448)
					   << QSize(864, 480)
					   << QSize(800, 600)
					   << QSize(960, 544)
					   << QSize(1024, 576)
					   << QSize(960, 720)
					   << QSize(1184, 656)
					   << QSize(1280, 720)
					   << QSize(1280, 960)
					   << QSize(1920, 1080)
					   ;
//#endif
	}

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		return _resolutions.size();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		const auto& serverConfig = _nc->serverConfig();
		const auto& size = _resolutions[index.row()];
		if (!VirtualServerConfigEntity::isResolutionSupported(serverConfig, size))
			return QAbstractListModel::flags(index) ^ Qt::ItemIsEnabled;
		return QAbstractListModel::flags(index);
	}

	virtual QVariant data(const QModelIndex& index,
						  int role = Qt::DisplayRole) const
	{
		if (index.row() >= _resolutions.size())
			return QVariant();

		const auto& serverConfig = _nc->serverConfig();
		const auto& size = _resolutions[index.row()];

		switch (role)
		{
			case Qt::DisplayRole:
			{
				if (!VirtualServerConfigEntity::isResolutionSupported(serverConfig, size))
					return QString("%1x%2 (Not supported by server)").arg(size.width()).arg(
							   size.height());
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
	QSharedPointer<NetworkClient> _nc;
	QList<QSize> _resolutions;
};

///////////////////////////////////////////////////////////////////////

static int bitrateForResolution(const QSize& resolution, int defaultBitrate,
								const VirtualServerConfigEntity& serverConfig)
{
	QList<QPair<QSize, int> > dimBitrates;
	dimBitrates.append(qMakePair(QSize(1980, 1080), 350));
	dimBitrates.append(qMakePair(QSize(1280, 720), 250));
	dimBitrates.append(qMakePair(QSize(640, 360), 100));
	for (const auto& item : dimBitrates)
	{
		if (VirtualServerConfigEntity::isResolutionSupported(serverConfig, item.first)
				&& VirtualServerConfigEntity::isBitrateSupported(serverConfig, item.second)
				&& resolution.width() >= item.first.width()
				&& resolution.height() >= item.first.height())
		{
			return item.second;
		}
	}
	return defaultBitrate;
}

///////////////////////////////////////////////////////////////////////

VideoSettingsDialog::VideoSettingsDialog(const QSharedPointer<NetworkClient>&
		nc, QWidget* parent) :
	QDialog(parent),
	_nc(nc)
{
	_ui.setupUi(this);

	const auto& serverConfig = _nc->serverConfig();

	// Devices
	const auto infos = QCameraInfo::availableCameras();
	for (auto i = 0; i < infos.size(); ++i)
	{
		_ui.devices->addItem(QIcon(), infos[i].description(), infos[i].deviceName());
	}
	_ui.devices->addItem(QIcon(), tr("No camera (Viewer mode)"),
						 QVariant(QString()));

	// Quality / Bandwidth / Bitrate
	// 1 = 0,125 KByte/s
	// 1024 = 128 KByte/s
	_ui.qualityValue->setReadOnly(true);
	_ui.qualityValue->setMinimum(50);
	_ui.qualityValue->setMaximum(serverConfig.maxVideoBitrate);
	_ui.qualityValue->setValue(bitrateForResolution(QSize(0, 0),
							   _ui.qualityValue->minimum(), serverConfig));

#if !defined(OCS_INCLUDE_OPENGL)
	_ui.hardwareAcceleration->setChecked(false);
	_ui.hardwareAcceleration->setVisible(false);
#endif

	// Ui events
	QObject::connect(_ui.devices,
					 static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
					 &VideoSettingsDialog::onCurrentDeviceIndexChanged);
	QObject::connect(_ui.resolutions,
					 static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
					 &VideoSettingsDialog::onCurrentResolutionIndexChanged);
	QObject::connect(_ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(_ui.cancelButton, &QPushButton::clicked, this,
					 &QDialog::reject);

	QWidgetUtil::resizeWidgetPerCent(this, 30, 30);
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
	//_ui.qualityValue->setValue(opts.cameraBitrate);

#if defined(OCS_INCLUDE_OPENGL)
	// Hardware acceleration
	_ui.hardwareAcceleration->setChecked(opts.uiVideoHardwareAccelerationEnabled);
#endif

	// Auto enable
	_ui.autoEnable->setChecked(opts.cameraAutoEnable);
}

const ConferenceVideoWindow::Options& VideoSettingsDialog::values()
{
	_opts.cameraDeviceId = _ui.devices->currentData().toString();
	_opts.cameraResolution = _ui.resolutions->currentData().toSize();
	_opts.cameraBitrate = _ui.qualityValue->value();
	_opts.uiVideoHardwareAccelerationEnabled =
		_ui.hardwareAcceleration->isChecked();
	_opts.cameraAutoEnable = _ui.autoEnable->isChecked();
	return _opts;
}

void VideoSettingsDialog::onCurrentDeviceIndexChanged(int index)
{
	const auto& serverConfig = _nc->serverConfig();

	const auto deviceId = _ui.devices->currentData().toString();
	_ui.resolutions->setEnabled(!deviceId.isEmpty());
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
		_ui.resolutions->setModel(new ResolutionListModel(_nc, cameraInfo, this));
	}
}

// Updates the bitrate to a predefined value, based on the choosen resolution
void VideoSettingsDialog::onCurrentResolutionIndexChanged(int index)
{
	const auto& serverConfig = _nc->serverConfig();
	const auto resolution = _ui.resolutions->itemData(index).toSize();
	const int bitrate = bitrateForResolution(resolution,
						_ui.qualityValue->minimum(), serverConfig);
	_ui.qualityValue->setValue(bitrate);
}
