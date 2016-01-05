#include "conferencevideowindow.h"

#include <QScopedPointer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCameraInfo>
#include <QHostInfo>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QWeakPointer>
#include <QHostAddress>
#include <QBoxLayout>
#include <QStatusBar>
#include <QLabel>
#include <QMenuBar>

#include <QtMultimedia/QCamera>

#include "humblelogging/api.h"

#include "qtasync.h"

#include "qcorreply.h"
#include "qcorframe.h"

#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"
#include "networkusageentity.h"
#include "jsonprotocolhelper.h"

#include "networkclient/clientlistmodel.h"
#include "util/qwidgetutil.h"
#include "clientcameravideowidget.h"
#include "remoteclientvideowidget.h"
#include "tileviewwidget.h"

#include "video/conferencevideowindowsidebar.h"
#include "video/videosettingswidget.h"

#if defined(OCS_INCLUDE_AUDIO)
#include <QAudioDeviceInfo>
#include "audio/audioframegrabber.h"
#include "audio/audioframeplayer.h"
#endif

HUMBLE_LOGGER(HL, "client.logic");

///////////////////////////////////////////////////////////////////////
// Local Helpers
///////////////////////////////////////////////////////////////////////

static QSharedPointer<QCamera> createCameraFromOptions(const ConferenceVideoWindow::Options& opts)
{
	auto cameraInfo = QCameraInfo::defaultCamera();
	foreach (auto ci, QCameraInfo::availableCameras())
	{
		if (ci.deviceName() == opts.cameraDeviceId)
		{
			cameraInfo = ci;
			break;
		}
	}
	return QSharedPointer<QCamera>(new QCamera(cameraInfo));
}

#if defined(OCS_INCLUDE_AUDIO)
static QAudioFormat createAudioFormat()
{
	QAudioFormat format;
	format.setSampleRate(8000);
	format.setChannelCount(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::UnSignedInt);
	return format;
}

static QSharedPointer<QAudioInput> createMicrophoneFromOptions(const ConferenceVideoWindow::Options& opts)
{
	auto info = QAudioDeviceInfo::defaultInputDevice();
	foreach (auto item, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
	{
		if (item.deviceName() == opts.audioInputDeviceId)
		{
			info = item;
			break;
		}
	}
	auto format = createAudioFormat();
	if (!info.isFormatSupported(format))
	{
		return QSharedPointer<QAudioInput>();
	}
	return QSharedPointer<QAudioInput>(new QAudioInput(info, format));
}
#endif

///////////////////////////////////////////////////////////////////////

ConferenceVideoWindow::ConferenceVideoWindow(const Options& opts, const QSharedPointer<NetworkClient>& nc, QWidget* parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags),
	_opts(opts),
	_networkClient(nc),
	_camera(),
	_sidebar(nullptr),
	_view(nullptr),
	_statusbar(nullptr)//(new QStatusBar())
{
	// GUI STUFF

	// Menu
	auto menuBar = new QMenuBar(this);
	setMenuBar(menuBar);

	auto menu = menuBar->addMenu(QIcon(), tr("Settings"));
	auto videoSettingsAction = menu->addAction(QIcon(), tr("Video..."));
	QObject::connect(videoSettingsAction, &QAction::triggered, this, &ConferenceVideoWindow::onActionVideoSettingsTriggered);

	// Status bar
	setStatusBar(_statusbar);

	// Central container widget
	auto w = new QWidget(this);
	auto l = new QBoxLayout(QBoxLayout::LeftToRight);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	w->setLayout(l);
	setCentralWidget(w);

	// Sidebar with actions.
	_sidebar = new ConferenceVideoWindowSidebar(this);
	l->addWidget(_sidebar, 0);

	// Central view widget.
	_view = new TileViewWidget(this, this);
	_view->setClientListModel(_networkClient->clientModel());
	l->addWidget(_view, 1);

	// Network usage statistics inside statusbar
	if (_statusbar)
	{
		auto bandwidthContainer = new QWidget();
		auto bandwidthContainerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
		bandwidthContainerLayout->setContentsMargins(3, 3, 3, 3);
		bandwidthContainer->setLayout(bandwidthContainerLayout);

		auto bandwidthRead = new QLabel("D: 0.0 KB/s");
		bandwidthRead->setObjectName("bandwidthRead");
		bandwidthContainerLayout->addWidget(bandwidthRead);

		auto bandwidthWrite = new QLabel("U: 0.0 KB/s");
		bandwidthWrite->setObjectName("bandwidthWrite");
		bandwidthContainerLayout->addWidget(bandwidthWrite);

		_statusbar->addWidget(bandwidthContainer, 1);

		QObject::connect(nc.data(), &NetworkClient::networkUsageUpdated, [this, bandwidthRead, bandwidthWrite](const NetworkUsageEntity & networkUsage)
		{
			bandwidthRead->setText(QString("D: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead)));
			bandwidthWrite->setText(QString("U: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite)));
			if (bandwidthRead->parentWidget())
			{
				bandwidthRead->parentWidget()->setToolTip(tr("Received: %1\nSent: %2")
						.arg(ELWS::humanReadableSize(networkUsage.bytesRead))
						.arg(ELWS::humanReadableSize(networkUsage.bytesWritten)));
			}
		});
	}


	// Geometry
	QWidgetUtil::resizeWidgetPerCent(this, 75.0, 75.0);
	QSettings settings;
	restoreGeometry(settings.value("UI/ClientApp-Geometry").toByteArray());

	// NON GUI STUFF

	connect(_networkClient.data(), &NetworkClient::error, this, &ConferenceVideoWindow::onError);
	connect(_networkClient.data(), &NetworkClient::serverError, this, &ConferenceVideoWindow::onServerError);
	connect(_networkClient.data(), &NetworkClient::clientJoinedChannel, this, &ConferenceVideoWindow::onClientJoinedChannel);
	connect(_networkClient.data(), &NetworkClient::clientLeftChannel, this, &ConferenceVideoWindow::onClientLeftChannel);
	connect(_networkClient.data(), &NetworkClient::clientDisconnected, this, &ConferenceVideoWindow::onClientDisconnected);
	connect(_networkClient.data(), &NetworkClient::newVideoFrame, this, &ConferenceVideoWindow::onNewVideoFrame);

	// Create QCamera by device ID.
	applyVideoInputOptions(_opts);

#if defined(OCS_INCLUDE_AUDIO)
	// Create QAudioInput (microphone).
	if (!_opts.audioInputDeviceId.isEmpty())
	{
		_audioInput = createMicrophoneFromOptions(_opts);

		auto grabber = new AudioFrameGrabber(_audioInput, this);
		QObject::connect(grabber, &AudioFrameGrabber::newFrame, [this](const PcmFrameRefPtr & f)
		{
			_networkClient->sendAudioFrame(f);
		});
	}

	// Create QAudioOutput (headphones).
	if (true)
	{
		_audioPlayer = QSharedPointer<AudioFramePlayer>(new AudioFramePlayer());
		_audioPlayer->setDeviceInfo(QAudioDeviceInfo::defaultOutputDevice());
		_audioPlayer->setFormat(createAudioFormat());
		QObject::connect(_networkClient.data(), &NetworkClient::newAudioFrame, [this](PcmFrameRefPtr f, int senderId)
		{
			_audioPlayer->add(f, senderId);
		});
	}
#endif

	// Create initial tiles.
	auto m = _networkClient->clientModel();
	for (auto i = 0; i < m->rowCount(); ++i)
	{
		auto c = m->data(m->index(i), ClientListModel::ClientEntityRole).value<ClientEntity>();
		onClientJoinedChannel(c, ChannelEntity());
	}

	// Auto turn ON camera.
	if (_camera && _opts.cameraAutoEnable)
	{
		_sidebar->setVideoEnabled(true);
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Auto turn ON microphone.
	if (_audioInput && _opts.audioInputAutoEnable)
	{
		TileViewWidget* tvw = nullptr;
		if ((tvw = dynamic_cast<TileViewWidget*>(_view)) != nullptr)
		{
			tvw->setAudioInputEnabled(true);
		}
	}
#endif
}

ConferenceVideoWindow::~ConferenceVideoWindow()
{
	if (_networkClient)
	{
		_networkClient->disconnect(this);
	}
	if (_view)
	{
		delete _view;
	}
	if (_camera)
	{
		_camera->stop();
	}
#if defined(OCS_INCLUDE_AUDIO)
	if (_audioInput)
	{
		_audioInput->stop();
	}
#endif
}

const ConferenceVideoWindow::Options& ConferenceVideoWindow::options() const
{
	return _opts;
}

void ConferenceVideoWindow::loadOptionsFromConfig(Options& opts) const
{
	QSettings s;
	opts.cameraDeviceId = s.value("Video/InputDeviceId", opts.cameraDeviceId).toString();
	opts.cameraAutoEnable = s.value("Video/InputDeviceAutoEnable", opts.cameraAutoEnable).toBool();
	opts.cameraResolution = s.value("Video/InputDeviceResolution", opts.cameraResolution).toSize();
	opts.cameraBitrate = s.value("Video/InputDeviceBitrate", opts.cameraBitrate).toInt();
}

void ConferenceVideoWindow::saveOptionsToConfig(const Options& opts) const
{
	QSettings s;
	s.setValue("Video/InputDeviceId", opts.cameraDeviceId);
	s.setValue("Video/InputDeviceAutoEnable", opts.cameraAutoEnable);
	s.setValue("Video/InputDeviceResolution", opts.cameraResolution);
	s.setValue("Video/InputDeviceBitrate", opts.cameraBitrate);
}

QSharedPointer<NetworkClient> ConferenceVideoWindow::networkClient() const
{
	return _networkClient;
}

QSharedPointer<QCamera> ConferenceVideoWindow::camera() const
{
	return _camera;
}

#if defined(OCS_INCLUDE_AUDIO)
QSharedPointer<QAudioInput> ConferenceVideoWindow::audioInput()
{
	return _audioInput;
}
#endif

void ConferenceVideoWindow::onActionVideoSettingsTriggered()
{
	QScopedPointer<VideoSettingsDialog> dialog(new VideoSettingsDialog(this));
	dialog->setModal(true);
	dialog->preselect(_opts);
	if (dialog->exec() != QDialog::Accepted)
	{
		return;
	}
	_opts = dialog->values();
	applyVideoInputOptions(_opts);
}

void ConferenceVideoWindow::onError(QAbstractSocket::SocketError socketError)
{
	HL_INFO(HL, QString("Socket error (error=%1; message=%2)").arg(socketError).arg(_networkClient->socket()->errorString()).toStdString());
	showError(tr("Network socket error."), _networkClient->socket()->errorString());
}

void ConferenceVideoWindow::onServerError(int code, const QString& message)
{
	HL_INFO(HL, QString("Server error (error=%1; message=%2)").arg(code).arg(message).toStdString());
	showError(tr("Server error."),  QString("%1: %2").arg(code).arg(message));
}

void ConferenceVideoWindow::onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	HL_INFO(HL, QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
	if (client.id != _networkClient->clientEntity().id)
	{
		_view->addClient(client, channel);
	}
}

void ConferenceVideoWindow::onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	HL_INFO(HL, QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
	_view->removeClient(client, channel);
}

void ConferenceVideoWindow::onClientDisconnected(const ClientEntity& client)
{
	HL_INFO(HL, QString("Client disconnected (client-id=%1)").arg(client.id).toStdString());
	_view->removeClient(client, ChannelEntity());
}

void ConferenceVideoWindow::onNewVideoFrame(YuvFrameRefPtr frame, int senderId)
{
	_view->updateClientVideo(frame, senderId);
}

void ConferenceVideoWindow::applyVideoInputOptions(const Options& opts)
{
	// Device
	if (_camera)
	{
		_camera->stop();
		_camera->unload();
		_camera.clear();
	}
	_camera = createCameraFromOptions(opts);
	emit cameraChanged();
}

void ConferenceVideoWindow::closeEvent(QCloseEvent* e)
{
	QSettings settings;
	settings.setValue("UI/ClientApp-Geometry", saveGeometry());

	auto reply = networkClient()->goodbye();
	QCORREPLY_AUTODELETE(reply);
}

void ConferenceVideoWindow::showResponseError(int status, const QString& errorMessage, const QString& details)
{
	HL_ERROR(HL, QString("Network response error (status=%1; message=%2)").arg(status).arg(errorMessage).toStdString());
	QMessageBox box(this);
	box.setWindowTitle(tr("Warning"));
	box.setIcon(QMessageBox::Warning);
	box.addButton(QMessageBox::Ok);
	box.setText(QString::number(status) + QString(": ") + errorMessage);
	if (!details.isEmpty())
		box.setDetailedText(details);
	box.setMinimumWidth(400);
	box.exec();
}

void ConferenceVideoWindow::showError(const QString& shortText, const QString& longText)
{
	HL_ERROR(HL, QString("%1: %2").arg(shortText).arg(longText).toStdString());
	QMessageBox box(this);
	box.setWindowTitle(tr("Warning"));
	box.setIcon(QMessageBox::Warning);
	box.addButton(QMessageBox::Ok);
	box.setText(shortText);
	box.setDetailedText(longText);
	box.setMinimumWidth(400);
	box.exec();
}