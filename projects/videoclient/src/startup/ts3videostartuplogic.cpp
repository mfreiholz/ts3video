#include "ts3videostartuplogic.h"

#include <QApplication>
#include <QProgressDialog>
#include <QUrl>
#include <QUrlQuery>
#include <QHostInfo>
#include <QThread>

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

#include "humblelogging/api.h"

#include "qtasync/src/qtasync.h"

#include "videolib/src/ts3video.h"
#include "videolib/src/elws.h"
#include "videolib/src/jsonprotocolhelper.h"
#include "videolib/src/cliententity.h"
#include "videolib/src/channelentity.h"

#include "../startupwidget.h"
#include "../clientapplogic.h"
#include "../ts3video/ts3videoentities.h"
#include "../ts3video/ts3videoupdatedialog.h"

HUMBLE_LOGGER(HL, "client");

// Endpoint to check for updates.
// e.g.: %1 = 0.2
const static QString versionCheckUrl = QString("http://api.mfreiholz.de/ts3video/1.0/versioncheck/%1").arg(IFVS_SOFTWARE_VERSION);

// Endpoint to lookup conference server.
// e.g.: %1 = TeamSpeak address    = teamspeak.insanefactory.com
//       %2 = TeamSpeak port       = 9987
//       %3 = TeamSpeak channel id = 34
//
// TODO Pass these settings as an encrypted value. It shouldn't be too easy
//      to guess the parameters and make use of them.
const static QString serverLookupUrl = QString("http://api.mfreiholz.de/ts3video/1.0/lookup/%1:%2:%3:%4");

///////////////////////////////////////////////////////////////////////

/*!
	Runs the basic application.

	Logic
	-----
	- Checks for updates.
	- Ask master server for route.
	- Connect to conference server (videoserver.exe).
	- Authenticates with conference server.
	- Joins conference room.
	- Sends and receives video streams.

	Some sample commands (obsolete)
	-------------------------------
	--server-address 127.0.0.1 --server-port 13370 --username TEST --channel-identifier default
	--server-address 127.0.0.1 --server-port  13370  --username  "iF.Manuel"  --channel-identifier  "127.0.0.1#9987#1#"  --channel-password  "2e302e302e373231"  --ts3-client-database-id  2  --skip-startup-dialog
	--server-address teamspeak.insanefactory.com --server-port  13370  --username  "iF.Manuel"  --channel-identifier  "127.0.0.1#9987#1#"  --channel-password  "2e302e302e373231"  --ts3-client-database-id  2  --skip-startup-dialog

	Parameters (--mode ts3video):
		--address       The address of the TeamSpeak server.
		--port	        The port of the TeamSpeak server.
		--channelid     The channel-id of the TeamSpeak server.
		--data          Encrypted parameters.

	Commands:
		--mode ts3video --address teamspeak.insanefactory.com --port 9987 --channelid 3
*/
Ts3VideoStartupLogic::Ts3VideoStartupLogic(QApplication* a) :
	QDialog(nullptr), AbstractStartupLogic(a)
{
	a->setQuitOnLastWindowClosed(true);
	_ui.setupUi(this);
	QObject::connect(this, &Ts3VideoStartupLogic::newProgress, this, &Ts3VideoStartupLogic::showProgress);
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
	_ts3opts.ts3ServerAddress = ELWS::getArgsValue("--address").toString();
	_ts3opts.ts3ServerPort = ELWS::getArgsValue("--port").toString().toULongLong();
	_ts3opts.ts3ChannelId = ELWS::getArgsValue("--channelid").toString().toULongLong();

	// TODO Load options from single encrypted argument.
	//auto enc = ELWS::getArgsValue("--data").toString();

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
	HL_ERROR(HL, QString("%1: %2 => %3").arg(QString::number(status)).arg(errorMessage).arg(details).toStdString());
	showProgress(QString("<font color=red>ERROR:</font> ") + QString::number(status) + QString(": ") + errorMessage);
}

void Ts3VideoStartupLogic::showError(const QString& shortText, const QString& longText)
{
	HL_ERROR(HL, QString("%1 => %2").arg(shortText).arg(longText).toStdString());
	showProgress(QString("<font color=red>ERROR:</font> ") + shortText);
}

void Ts3VideoStartupLogic::start()
{
	if (!checkVersion())
	{
		quitDelayed();
		return;
	}
	if (!lookupVideoServer())
	{
		return;
	}
	initNetwork();
}

bool Ts3VideoStartupLogic::checkVersion()
{
	showProgress(tr("Checking for plugin updates..."));

	QEventLoop loop;
	QNetworkAccessManager mgr;
	auto reply = mgr.get(QNetworkRequest(QUrl(versionCheckUrl)));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
		showError(tr("Update check failed"), reply->errorString());
		return true;
	}

	auto data = reply->readAll();
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		showError(err.errorString(), QString::fromUtf8(data));
		return true;
	}

	QList<VersionInfo> versions;
	auto jversions = doc.array();
	for (auto i = 0; i < jversions.size(); ++i)
	{
		auto jv = jversions.at(i).toObject();
		VersionInfo vi;
		vi.fromJson(jv);
		versions.append(vi);
	}

	if (versions.isEmpty())
	{
		showProgress(tr("No updates available."));
		return true;
	}

	showProgress(tr("<font color=green>Updates available!</font>"));
	Ts3VideoUpdateDialog dlg;
	dlg.setVersions(versions);
	if (dlg.exec() == QDialog::Accepted)
	{
		return false; // Abort: User choosed update-now.
	}

	return true;
}

bool Ts3VideoStartupLogic::lookupVideoServer()
{
	// Lookup video server on master server.
	showProgress(tr("Looking for conference on master server..."));

	QEventLoop loop;
	QNetworkAccessManager mgr;
	auto url = serverLookupUrl.arg(_ts3opts.ts3ServerAddress).arg(_ts3opts.ts3ServerPort).arg(_ts3opts.ts3ChannelId).arg(_ts3opts.ts3ClientDbId);
	auto reply = mgr.get(QNetworkRequest(QUrl(url)));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
		showError(tr("Conference lookup failed"), reply->errorString());
		return false;
	}

	auto data = reply->readAll();
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		showError(err.errorString(), QString::fromUtf8(data));
		return false;
	}

	ConferenceJoinInfo info;
	if (!info.fromJson(doc.object()))
	{
		showError("Received invalid conference information", QString::fromUtf8(data));
		return false;
	}
	return true;
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
	showProgress(tr("Resolve DNS for %1").arg(_opts.serverAddress));
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
	showProgress(tr("Authenticating..."));
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