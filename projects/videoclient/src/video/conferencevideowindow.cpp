#include "conferencevideowindow_p.h"

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
#endif

HUMBLE_LOGGER(HL, "client.logic");

///////////////////////////////////////////////////////////////////////

static ConferenceVideoWindow* gFirstInstance;

ConferenceVideoWindow* ConferenceVideoWindow::instance()
{
	return gFirstInstance;
}

///////////////////////////////////////////////////////////////////////

ConferenceVideoWindow::ConferenceVideoWindow(const Options& opts, const QSharedPointer<NetworkClient>& nc, QWidget* parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags),
	d(new ConferenceVideoWindowPrivate(this))
{
	if (!gFirstInstance)
		gFirstInstance = this;

	d->opts = opts;
	d->nc = nc;

	connect(d->nc.data(), &NetworkClient::error, this, &ConferenceVideoWindow::onError);
	connect(d->nc.data(), &NetworkClient::serverError, this, &ConferenceVideoWindow::onServerError);
	connect(d->nc.data(), &NetworkClient::clientJoinedChannel, this, &ConferenceVideoWindow::onClientJoinedChannel);
	connect(d->nc.data(), &NetworkClient::clientLeftChannel, this, &ConferenceVideoWindow::onClientLeftChannel);
	connect(d->nc.data(), &NetworkClient::clientDisconnected, this, &ConferenceVideoWindow::onClientDisconnected);
	connect(d->nc.data(), &NetworkClient::newVideoFrame, this, &ConferenceVideoWindow::onNewVideoFrame);

	// Central view widget.
	auto viewWidget = new TileViewWidget(this);
	viewWidget->setClientListModel(d->nc->clientModel());
	d->view = viewWidget;
	setCentralWidget(viewWidget);

	// Create QCamera by device ID.
	if (!d->opts.cameraDeviceId.isEmpty())
	{
		d->camera = d->createCameraFromOptions();
		viewWidget->setCamera(d->camera);
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Create QAudioInput (microphone).
	if (!d->opts.audioInputDeviceId.isEmpty())
	{
		d->audioInput = d->createMicrophoneFromOptions();

		auto grabber = new AudioFrameGrabber(d->audioInput, this);
		QObject::connect(grabber, &AudioFrameGrabber::newFrame, [this](const PcmFrameRefPtr & f)
		{
			d->nc->sendAudioFrame(f);
		});
	}

	// Create QAudioOutput (headphones).
	if (true)
	{
		d->audioPlayer = QSharedPointer<AudioFramePlayer>(new AudioFramePlayer());
		d->audioPlayer->setDeviceInfo(QAudioDeviceInfo::defaultOutputDevice());
		d->audioPlayer->setFormat(d->createAudioFormat());
		QObject::connect(d->nc.data(), &NetworkClient::newAudioFrame, [this](PcmFrameRefPtr f, int senderId)
		{
			d->audioPlayer->add(f, senderId);
		});
	}
#endif

	// Create initial tiles.
	auto m = d->nc->clientModel();
	for (auto i = 0; i < m->rowCount(); ++i)
	{
		auto c = m->data(m->index(i), ClientListModel::ClientEntityRole).value<ClientEntity>();
		onClientJoinedChannel(c, ChannelEntity());
	}

	// Auto turn ON camera.
	if (d->camera && d->opts.cameraAutoEnable)
	{
		TileViewWidget* tvw = nullptr;
		if ((tvw = dynamic_cast<TileViewWidget*>(d->view)) != nullptr)
		{
			tvw->setVideoEnabled(true);
		}
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Auto turn ON microphone.
	if (d->audioInput && d->opts.audioInputAutoEnable)
	{
		TileViewWidget* tvw = nullptr;
		if ((tvw = dynamic_cast<TileViewWidget*>(d->view)) != nullptr)
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
	if (d->nc)
	{
		d->nc->disconnect(this);
	}
	if (d->view)
	{
		delete d->view;
	}
	if (d->camera)
	{
		d->camera->stop();
	}
#if defined(OCS_INCLUDE_AUDIO)
	if (d->audioInput)
	{
		d->audioInput->stop();
	}
#endif
}

QSharedPointer<NetworkClient> ConferenceVideoWindow::networkClient()
{
	return d->nc;
}

#if defined(OCS_INCLUDE_AUDIO)
QSharedPointer<QAudioInput> ConferenceVideoWindow::audioInput()
{
	return d->audioInput;
}
#endif

void ConferenceVideoWindow::onError(QAbstractSocket::SocketError socketError)
{
	HL_INFO(HL, QString("Socket error (error=%1; message=%2)").arg(socketError).arg(d->nc->socket()->errorString()).toStdString());
	showError(tr("Network socket error."), d->nc->socket()->errorString());
}

void ConferenceVideoWindow::onServerError(int code, const QString& message)
{
	HL_INFO(HL, QString("Server error (error=%1; message=%2)").arg(code).arg(message).toStdString());
	showError(tr("Server error."),  QString("%1: %2").arg(code).arg(message));
}

void ConferenceVideoWindow::onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	HL_INFO(HL, QString("Client joined channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
	if (client.id != d->nc->clientEntity().id)
	{
		d->view->addClient(client, channel);
	}
}

void ConferenceVideoWindow::onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel)
{
	HL_INFO(HL, QString("Client left channel (client-id=%1; channel-id=%2)").arg(client.id).arg(channel.id).toStdString());
	d->view->removeClient(client, channel);
}

void ConferenceVideoWindow::onClientDisconnected(const ClientEntity& client)
{
	HL_INFO(HL, QString("Client disconnected (client-id=%1)").arg(client.id).toStdString());
	d->view->removeClient(client, ChannelEntity());
}

void ConferenceVideoWindow::onNewVideoFrame(YuvFrameRefPtr frame, int senderId)
{
	d->view->updateClientVideo(frame, senderId);
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

///////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////

ConferenceVideoWindowPrivate::ConferenceVideoWindowPrivate(ConferenceVideoWindow* o) :
	QObject(o), owner(o), opts(), view(nullptr), cameraWidget(nullptr)
{
}

ConferenceVideoWindowPrivate::~ConferenceVideoWindowPrivate()
{
}

QSharedPointer<QCamera> ConferenceVideoWindowPrivate::createCameraFromOptions() const
{
	auto d = this;
	auto cameraInfo = QCameraInfo::defaultCamera();
	foreach (auto ci, QCameraInfo::availableCameras())
	{
		if (ci.deviceName() == d->opts.cameraDeviceId)
		{
			cameraInfo = ci;
			break;
		}
	}
	return QSharedPointer<QCamera>(new QCamera(cameraInfo));
}

#if defined(OCS_INCLUDE_AUDIO)
QSharedPointer<QAudioInput> ConferenceVideoWindowPrivate::createMicrophoneFromOptions() const
{
	auto d = this;
	auto info = QAudioDeviceInfo::defaultInputDevice();
	foreach (auto item, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
	{
		if (item.deviceName() == d->opts.audioInputDeviceId)
		{
			info = item;
			break;
		}
	}
	auto format = d->createAudioFormat();
	if (!info.isFormatSupported(format))
	{
		return QSharedPointer<QAudioInput>();
	}
	return QSharedPointer<QAudioInput>(new QAudioInput(info, format));
}
#endif