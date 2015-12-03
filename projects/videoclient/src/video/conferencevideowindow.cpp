#include "conferencevideowindow.h"

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

#if defined(OCS_INCLUDE_AUDIO)
#include <QAudioDeviceInfo>
#include "audio/audioframegrabber.h"
#include "audio/audioframeplayer.h"
#endif

HUMBLE_LOGGER(HL, "client.logic");

///////////////////////////////////////////////////////////////////////

static ConferenceVideoWindow* gFirstInstance;

ConferenceVideoWindow* ConferenceVideoWindow::instance()
{
	return gFirstInstance;
}

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
	_view(nullptr)
{
	if (!gFirstInstance)
		gFirstInstance = this;

	connect(_networkClient.data(), &NetworkClient::error, this, &ConferenceVideoWindow::onError);
	connect(_networkClient.data(), &NetworkClient::serverError, this, &ConferenceVideoWindow::onServerError);
	connect(_networkClient.data(), &NetworkClient::clientJoinedChannel, this, &ConferenceVideoWindow::onClientJoinedChannel);
	connect(_networkClient.data(), &NetworkClient::clientLeftChannel, this, &ConferenceVideoWindow::onClientLeftChannel);
	connect(_networkClient.data(), &NetworkClient::clientDisconnected, this, &ConferenceVideoWindow::onClientDisconnected);
	connect(_networkClient.data(), &NetworkClient::newVideoFrame, this, &ConferenceVideoWindow::onNewVideoFrame);

	// Central view widget.
	auto viewWidget = new TileViewWidget(this);
	viewWidget->setClientListModel(_networkClient->clientModel());
	_view = viewWidget;
	setCentralWidget(viewWidget);

	// Create QCamera by device ID.
	if (!_opts.cameraDeviceId.isEmpty())
	{
		_camera = createCameraFromOptions(_opts);
		viewWidget->setCamera(_camera);
	}

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
		TileViewWidget* tvw = nullptr;
		if ((tvw = dynamic_cast<TileViewWidget*>(_view)) != nullptr)
		{
			tvw->setVideoEnabled(true);
		}
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

	QWidgetUtil::resizeWidgetPerCent(this, 75.0, 75.0);

	QSettings settings;
	restoreGeometry(settings.value("UI/ClientApp-Geometry").toByteArray());
}

ConferenceVideoWindow::~ConferenceVideoWindow()
{
	if (gFirstInstance == this)
	{
		gFirstInstance = nullptr;
	}
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

QSharedPointer<NetworkClient> ConferenceVideoWindow::networkClient()
{
	return _networkClient;
}

#if defined(OCS_INCLUDE_AUDIO)
QSharedPointer<QAudioInput> ConferenceVideoWindow::audioInput()
{
	return _audioInput;
}
#endif

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