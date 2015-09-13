#include "ts3videostartuplogic.h"

#include <QApplication>
#include <QProgressDialog>
#include <QUrl>
#include <QUrlQuery>
#include <QHostInfo>
#include <QThread>

#include "humblelogging/api.h"

#include "qtasync/src/qtasync.h"

#include "videolib/src/ts3video.h"
#include "videolib/src/elws.h"
#include "videolib/src/jsonprotocolhelper.h"
#include "videolib/src/cliententity.h"
#include "videolib/src/channelentity.h"

#include "../startupwidget.h"
#include "../clientapplogic.h"

HUMBLE_LOGGER(HL, "client");

///////////////////////////////////////////////////////////////////////

/*!
    Runs the basic application.

    Logic
    -----
    - Connects to server.
    - Authenticates with server.
    - Joins channel.
    - Sends and receives video streams.

    URL Syntax example
    ------------------
    By using the "--uri" parameter its possible to define those parameters with a URI.
    e.g.:
    ts3video://127.0.0.1:6000/?username=mfreiholz&password=secret1234&channelid=42&...

    Some sample commands
    --------------------
    --server-address 127.0.0.1 --server-port 13370 --username TEST --channel-identifier default
    --server-address 127.0.0.1 --server-port  13370  --username  "iF.Manuel"  --channel-identifier  "127.0.0.1#9987#1#"  --channel-password  "2e302e302e373231"  --ts3-client-database-id  2  --skip-startup-dialog
    --server-address teamspeak.insanefactory.com --server-port  13370  --username  "iF.Manuel"  --channel-identifier  "127.0.0.1#9987#1#"  --channel-password  "2e302e302e373231"  --ts3-client-database-id  2  --skip-startup-dialog
*/
Ts3VideoStartupLogic::Ts3VideoStartupLogic(QApplication* a) :
	QDialog(nullptr), AbstractStartupLogic(a)
{
	_ui.setupUi(this);
	a->setQuitOnLastWindowClosed(true);
}

Ts3VideoStartupLogic::~Ts3VideoStartupLogic()
{
	if (_nc)
	{
		_nc->disconnect(this);
		_nc.clear();
	}
}

int Ts3VideoStartupLogic::exec()
{
	setVisible(true);

	// Load options from arguments.
	ClientAppLogic::Options opts;
	opts.serverAddress = ELWS::getArgsValue("--server-address", opts.serverAddress).toString();
	opts.serverPort = ELWS::getArgsValue("--server-port", opts.serverPort).toUInt();
	opts.serverPassword = ELWS::getArgsValue("--server-password", opts.serverPassword).toString();
	opts.username = ELWS::getArgsValue("--username", ELWS::getUserName()).toString();
	opts.channelId = ELWS::getArgsValue("--channel-id", opts.channelId).toLongLong();
	opts.channelIdentifier = ELWS::getArgsValue("--channel-identifier", opts.channelIdentifier).toString();
	opts.channelPassword = ELWS::getArgsValue("--channel-password", opts.channelPassword).toString();
	opts.authParams.insert("ts3_client_database_id", ELWS::getArgsValue("--ts3-client-database-id", 0).toULongLong());

	// Load options from URI.
	QUrl url(ELWS::getArgsValue("--uri").toString(), QUrl::StrictMode);
	if (url.isValid())
	{
		QUrlQuery urlQuery(url);
		opts.serverAddress = url.host();
		opts.serverPort = url.port(opts.serverPort);
		opts.serverPassword = urlQuery.queryItemValue("serverpassword");
		opts.username = urlQuery.queryItemValue("username");
		opts.channelId = urlQuery.queryItemValue("channelid").toLongLong();
		opts.channelIdentifier = urlQuery.queryItemValue("channelidentifier");
		opts.channelPassword = urlQuery.queryItemValue("channelpassword");
		if (opts.username.isEmpty())
		{
			opts.username = ELWS::getUserName();
		}
	}

	// Modify startup options with dialog.
	// Skip dialog with: --skip-startup-dialog
	StartupDialog dialog(this);
	dialog.setValues(opts);
	if (!ELWS::hasArgsValue("--skip-startup-dialog"))
	{
		if (dialog.exec() != QDialog::Accepted)
		{
			quitDelayed();
			return AbstractStartupLogic::exec();
		}
	}
	opts = dialog.values();
	_opts = opts;

	HL_INFO(HL, QString("-- START (version=%1) -----").arg(qapp()->applicationVersion()).toStdString());
	HL_INFO(HL, QString("Address: %1").arg(opts.serverAddress).toStdString());
	HL_INFO(HL, QString("Port: %1").arg(opts.serverPort).toStdString());
	HL_INFO(HL, QString("Username: %1").arg(opts.username).toStdString());
	HL_INFO(HL, QString("Channel ID: %1").arg(opts.channelId).toStdString());
	HL_INFO(HL, QString("Channel Ident: %1").arg(opts.channelIdentifier).toStdString());
	HL_INFO(HL, QString("Camera device ID: %1").arg(opts.cameraDeviceId).toStdString());

	HL_INFO(HL, QString("-- CONNECT ----------------").toStdString());
	start();

	return AbstractStartupLogic::exec();
}

void Ts3VideoStartupLogic::showProgress(const QString& text)
{
	_ui.progress->append(text.trimmed() + QString("<br>\n"));
}

void Ts3VideoStartupLogic::showResponseError(int status, const QString& errorMessage, const QString& details)
{
	HL_ERROR(HL, details.toStdString());
	showProgress(QString("<font color=red>ERROR:</font> ") + QString::number(status) + QString(": ") + errorMessage);
}

void Ts3VideoStartupLogic::showError(const QString& shortText, const QString& longText)
{
	HL_ERROR(HL, longText.isEmpty() ? shortText.toStdString() : longText.toStdString());
	showProgress(QString("<font color=red>ERROR:</font> ") + shortText);
}

void Ts3VideoStartupLogic::start()
{
	lookupVideoServer();
}

void Ts3VideoStartupLogic::lookupVideoServer()
{
	// Lookup video server on master server.
	showProgress(tr("Lookup conference on master server..."));
	QtAsync::async([this]()
	{
		QThread::sleep(1);
		return QVariant();
	},
	[this](QVariant v)
	{
		this->initNetwork();
	});
}

void Ts3VideoStartupLogic::initNetwork()
{
	_nc = QSharedPointer<NetworkClient>(new NetworkClient());
	connect(_nc.data(), &NetworkClient::connected, this, &Ts3VideoStartupLogic::onConnected);
	connect(_nc.data(), &NetworkClient::disconnected, this, &Ts3VideoStartupLogic::onDisconnected);
	connect(_nc.data(), &NetworkClient::error, this, &Ts3VideoStartupLogic::onError);

	// If the address is already an IP, we don't need a lookup.
	auto address = QHostAddress(_opts.serverAddress);
	if (!address.isNull())
	{
		showProgress(tr("Connecting to server %1:%2 (address=%3)").arg(_opts.serverAddress).arg(_opts.serverPort).arg(address.toString()));
		_nc->connectToHost(address, _opts.serverPort);
		return;
	}

	// Async DNS lookup.
	showProgress(tr("DNS lookup for server %1").arg(_opts.serverAddress));
	QtAsync::async([this]()
	{
		auto hostInfo = QHostInfo::fromName(_opts.serverAddress);
		return QVariant::fromValue(hostInfo);
	},
	[this](QVariant v)
	{
		auto hostInfo = v.value<QHostInfo>();
		if (hostInfo.error() != QHostInfo::NoError || hostInfo.addresses().isEmpty())
		{
			showError(tr("DNS lookup failed"), hostInfo.errorString());
			return;
		}
		auto address = hostInfo.addresses().first();
		showProgress(tr("Connecting to server %1:%2 (address=%3)").arg(_opts.serverAddress).arg(_opts.serverPort).arg(address.toString()));
		_nc->connectToHost(address, _opts.serverPort);
	});
}

void Ts3VideoStartupLogic::authAndJoinConference()
{
	// Authenticate.
	showProgress(tr("Authenticate..."));
	auto reply = _nc->auth(_opts.username, _opts.serverPassword, _opts.authParams);
	QObject::connect(reply, &QCorReply::finished, [this, reply]()
	{
		HL_DEBUG(HL, QString("Auth answer: %1").arg(QString(reply->frame()->data())).toStdString());
		reply->deleteLater();

		int status;
		QString errorString;
		QJsonObject params;
		if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, errorString))
		{
			this->showError(tr("Protocol error"), reply->frame()->data());
			return;
		}
		else if (status != 0)
		{
			this->showResponseError(status, errorString, reply->frame()->data());
			return;
		}

		// Join channel.
		showProgress(tr("Joining conference..."));
		QCorReply* reply2 = nullptr;
		if (_opts.channelId != 0)
		{
			reply2 = _nc->joinChannel(_opts.channelId, _opts.channelPassword);
		}
		else
		{
			reply2 = _nc->joinChannelByIdentifier(_opts.channelIdentifier, _opts.channelPassword);
		}
		QObject::connect(reply2, &QCorReply::finished, [this, reply2]()
		{
			HL_DEBUG(HL, QString("Join channel answer: %1").arg(QString(reply2->frame()->data())).toStdString());
			reply2->deleteLater();

			int status;
			QString errorString;
			QJsonObject params;
			if (!JsonProtocolHelper::fromJsonResponse(reply2->frame()->data(), status, params, errorString))
			{
				this->showError(tr("Protocol error"), reply2->frame()->data());
				return;
			}
			else if (status != 0)
			{
				this->showResponseError(status, errorString, reply2->frame()->data());
				return;
			}
			this->startVideoGui();
		});
	});
}

void Ts3VideoStartupLogic::startVideoGui()
{
	auto w = new ClientAppLogic(_opts, _nc, nullptr, 0);
	w->resize(1024, 768);
	w->show();
	w->start();

	close();
	if (_nc)
	{
		_nc->disconnect(this);
		_nc.clear();
	}
}

void Ts3VideoStartupLogic::quitDelayed()
{
	QMetaObject::invokeMethod(qapp(), "quit", Qt::QueuedConnection);
}

void Ts3VideoStartupLogic::onConnected()
{
	authAndJoinConference();
}

void Ts3VideoStartupLogic::onDisconnected()
{
	showProgress(tr("Disconnected!"));
}

void Ts3VideoStartupLogic::onError(QAbstractSocket::SocketError socketError)
{
	showError(_nc->socket()->errorString());
}