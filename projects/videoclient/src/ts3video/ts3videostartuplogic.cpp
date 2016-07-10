#include "ts3videostartuplogic.h"

#include <QApplication>
#include <QProgressDialog>
#include <QUrl>
#include <QUrlQuery>
#include <QHostInfo>
#include <QThread>
#include <QTimer>

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

#include "qtasync/qtasync.h"

#include "videolib/ts3video.h"
#include "videolib/elws.h"
#include "videolib/jsonprotocolhelper.h"
#include "videolib/cliententity.h"
#include "videolib/channelentity.h"

#include "startup/startupwidget.h"
#include "video/conferencevideowindow.h"
#include "video/videosettingswidget.h"
#include "ts3video/ts3videoupdatedialog.h"
#include "util/qwidgetutil.h"

HUMBLE_LOGGER(HL, "client");

// Endpoint to check for updates.
// e.g.: %1 = 0.2
const static QString versionCheckUrl = QString("http://api.mfreiholz.de/ts3video/1.0/versioncheck/%1").arg(IFVS_SOFTWARE_VERSION);

// Endpoint to lookup conference server.
// e.g.: %1 = TeamSpeak address            = teamspeak.insanefactory.com
//       %2 = TeamSpeak port               = 9987
//       %3 = TeamSpeak channel id         = 34
//
// TODO Pass these settings as an encrypted value. It shouldn't be too easy
//      to guess the parameters and make use of them.
const static QString serverLookupUrl = QString("http://api.mfreiholz.de/ts3video/1.0/lookup/%1:%2:%3");

// Endpoint to lookup public conference server.
// e.g.: %1 = TeamSpeak address            = teamspeak.insanefactory.com
//       %2 = TeamSpeak port               = 9987
//       %3 = TeamSpeak channel id         = 34
const static QString serverLookupPublicUrl = QString("http://api.mfreiholz.de/ts3video/1.0/lookup-public/%1:%2:%3");

///////////////////////////////////////////////////////////////////////

static QString generateConferenceRoomPassword(const QString& uid)
{
	QString s;
	for (auto i = uid.size() / 2 - 1; i >= 0; i--)
	{
		const int ci = uid[i].toLatin1();
		s.append(QString::number(ci, 16));
	}
	return s;
}

/*!
	Runs the basic application.

	# Logic

	- Checks for updates.
	- Ask master server for route.
	- Connect to conference server (videoserver.exe).
	- Authenticates with conference server.
	- Joins conference room.
	- Sends and receives video streams.

	# Parameters (--mode ts3video)

	--address       The address of the TeamSpeak server.
	--port          The port of the TeamSpeak server.
	--channelid     The channel-id of the TeamSpeak server.
	--clientdbid    The clients database ID on TeamSpeak server.
	--username      The name of the TeamSpeak user.
	--public        Indicates that a public server should be used.

	--skip-update-check
		Skips the update check.
	--skip-server-lookup
		Skips the server lookup on master (does not work with --public flag).

	# Commands

	--mode ts3video --address teamspeak.insanefactory.com --port 9987 --channelid 1 --clientdbid 6 --username "Manuel"
	--mode ts3video --address 127.0.0.1 --port 9987 --channelid 1 --username "Manuel"
	--mode ts3video --address 127.0.0.1 --port 9987 --channelid 1 --username "Manuel" --skip-update-check --skip-server-lookup
*/
Ts3VideoStartupLogic::Ts3VideoStartupLogic(QApplication* a) :
	QDialog(nullptr), AbstractStartupLogic(a), _window(nullptr)
{
	// Setup application.
	a->setQuitOnLastWindowClosed(true);

	// Init GUI.
	_ui.setupUi(this);

	QWidgetUtil::resizeWidgetPerCent(this, 35.0, 35.0);
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
	_args.ts3ServerAddress = ELWS::getArgsValue("--address").toString();
	_args.ts3ServerPort = ELWS::getArgsValue("--port").toString().toULongLong();
	_args.ts3ChannelId = ELWS::getArgsValue("--channelid").toString().toULongLong();
	_args.ts3ClientDbId = ELWS::getArgsValue("--clientdbid").toString().toULongLong();
	_args.ts3Username = ELWS::getArgsValue("--username").toString();
	_args.usePublicConferenceServer = ELWS::hasArgsValue("--public");
	_args.skipUpdateCheck = ELWS::hasArgsValue("--skip-update-check");
	_args.skipServerLookup = ELWS::hasArgsValue("--skip-server-lookup");

	// TODO Load options from single encrypted argument.
	//auto enc = ELWS::getArgsValue("--data").toString();

	HL_INFO(HL, QString("-- START (version=%1) -----").arg(IFVS_SOFTWARE_VERSION).toStdString());
	HL_INFO(HL, QString("TS3-Address: %1").arg(_args.ts3ServerAddress).toStdString());
	HL_INFO(HL, QString("TS3-Port: %1").arg(_args.ts3ServerPort).toStdString());
	HL_INFO(HL, QString("TS3-Channel ID: %1").arg(_args.ts3ChannelId).toStdString());
	HL_INFO(HL, QString("TS3-Client DBID: %1").arg(_args.ts3ClientDbId).toStdString());
	HL_INFO(HL, QString("TS3-Username: %1").arg(_args.ts3Username).toStdString());

	start();

	const auto execCode = AbstractStartupLogic::exec();
	if (_window)
	{
		_window->saveOptionsToConfig(_window->options());
	}
	return execCode;
}

void Ts3VideoStartupLogic::setStatus(const QString& text)
{
	_ui.progress->append(QString("<br>") + text.trimmed().toHtmlEscaped() + QString("<br>"));
}

void Ts3VideoStartupLogic::setStatusInfo(const QString& text)
{
	_ui.progress->append(QString("<font color=blue>") + text.trimmed() + QString("</font><br>"));
}

void Ts3VideoStartupLogic::setStatusError(const QString& text, const QString& detail)
{
	QString s;
	s.append("<font color=red>");
	s.append(text.trimmed());
	s.append("</font>");
	if (!detail.isEmpty())
	{
		s.append("<br>");
		s.append("<font color=\"#aaaaaa\">");
		s.append(detail.trimmed().toHtmlEscaped());
		s.append("</font>");
	}
	s.append("<br>");
	_ui.progress->append(s);
}

void Ts3VideoStartupLogic::start()
{
	setStatus(tr("Initializing conference now"));

	// Check for updates.
	if (!_args.skipUpdateCheck)
	{
		if (!checkVersion())
		{
			quitDelayed();
			return;
		}
	}

	// Lookup dedicated server on "--address":default-port
	QEventLoop ev;
	NetworkClient nc;
	nc.connectToHost(QHostAddress(_args.ts3ServerAddress), 13370); // DNS lookup?
	//QObject::connect(&nc, &NetworkClient::connected, &ev, &QEventLoop::exit);
	//QObject::connect(&nc, &NetworkClient::error, &ev, &QEventLoop::exit);
	ev.exec();
	if (nc.socket()->state() == QAbstractSocket::ConnectedState)
	{
		QCorReply::autoDelete(nc.goodbye());
		QApplication::processEvents();
	}

	// Lookup conference.
	if (_args.usePublicConferenceServer)
	{
		if (!lookupPublicConference())
		{
			return;
		}
	}
	else
	{
		if (_args.skipServerLookup || !lookupConference())
		{
			// Lookup failed.
			// Fallback to old behavior and join own dedicated server.
			_joinInfo.server.address = _args.ts3ServerAddress;
			_joinInfo.server.port = 13370;
			_joinInfo.server.password = QString();
			_joinInfo.room.uid = QString("%1#%2#%3#").arg(_args.ts3ServerAddress).arg(QString::number(_args.ts3ServerPort)).arg(_args.ts3ChannelId);
			_joinInfo.room.password = generateConferenceRoomPassword(_joinInfo.room.uid);
		}
	}

	// Connect to conference.
	initNetwork(_joinInfo.server.address, _joinInfo.server.port);
}

bool Ts3VideoStartupLogic::checkVersion()
{
	setStatus(tr("Checking updates..."));

	QEventLoop loop;
	QNetworkAccessManager mgr;
	auto reply = mgr.get(QNetworkRequest(QUrl(versionCheckUrl)));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
		setStatusError(tr("Update check failed"), reply->errorString());
		return true;
	}

	auto data = reply->readAll();
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		setStatusError(tr("Update check failed"), QString("%1: %2").arg(err.errorString()).arg(QString::fromUtf8(data)));
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
		setStatusInfo(tr("Your software is up-to-date."));
		return true;
	}

	setStatusInfo(tr("Software updates available."));
	Ts3VideoUpdateDialog dlg;
	dlg.setVersions(versions);
	if (dlg.exec() == QDialog::Accepted)
	{
		return false; // Abort: User choosed update-now.
	}

	return true;
}

bool Ts3VideoStartupLogic::lookupConference()
{
	// Lookup video server on master server.
	setStatus(tr("Looking for conference on master server..."));

	QEventLoop loop;
	QNetworkAccessManager mgr;
	QUrlQuery query;
	query.addQueryItem("version", IFVS_SOFTWARE_VERSION);
	auto url = QUrl(serverLookupUrl.arg(_args.ts3ServerAddress).arg(_args.ts3ServerPort).arg(_args.ts3ChannelId));
	url.setQuery(query);
	auto reply = mgr.get(QNetworkRequest(url));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
		if (reply->error() == QNetworkReply::ContentNotFoundError)
		{
			setStatusInfo(tr("No conference server registered on master, falling back to self hosted server."));
			return false;
		}
		else
		{
			setStatusError(tr("Conference lookup failed"), QString("%1: %2").arg(reply->errorString()).arg(QString::fromUtf8(reply->readAll())));
			return false;
		}
	}

	const auto data = reply->readAll();
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		setStatusError(tr("Conference lookup failed"), QString("%1: %2").arg(err.errorString()).arg(QString::fromUtf8(data)));
		return false;
	}

	if (!_joinInfo.fromJson(doc.object()))
	{
		setStatusError(tr("Received invalid conference information"), QString::fromUtf8(data));
		return false;
	}
	return true;
}

bool Ts3VideoStartupLogic::lookupPublicConference()
{
	// Lookup video server on master server.
	setStatus(tr("Looking for public conference on master server..."));

	QEventLoop loop;
	QNetworkAccessManager mgr;
	QUrlQuery query;
	query.addQueryItem("version", IFVS_SOFTWARE_VERSION);
	auto url = QUrl(serverLookupPublicUrl.arg(_args.ts3ServerAddress).arg(_args.ts3ServerPort).arg(_args.ts3ChannelId));
	url.setQuery(query);
	auto reply = mgr.get(QNetworkRequest(url));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
		if (reply->error() == QNetworkReply::ContentNotFoundError)
		{
			setStatusInfo(tr("No conference server registered on master, falling back to self hosted server."));
			return false;
		}
		else
		{
			setStatusError(tr("Conference lookup failed"), QString("%1: %2").arg(reply->errorString()).arg(QString::fromUtf8(reply->readAll())));
			return false;
		}
	}

	auto data = reply->readAll();
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		setStatusError(tr("Conference lookup failed"), QString("%1: %2").arg(err.errorString()).arg(QString::fromUtf8(data)));
		return false;
	}

	if (!_joinInfo.fromJson(doc.object()))
	{
		setStatusError(tr("Received invalid conference information"), QString::fromUtf8(data));
		return false;
	}
	return true;
}

void Ts3VideoStartupLogic::initNetwork(const QString& address, quint16 port)
{
	_nc = QSharedPointer<NetworkClient>(new NetworkClient());
	connect(_nc.data(), &NetworkClient::connected, this, &Ts3VideoStartupLogic::onConnected);
	connect(_nc.data(), &NetworkClient::disconnected, this, &Ts3VideoStartupLogic::onDisconnected);
	connect(_nc.data(), &NetworkClient::error, this, &Ts3VideoStartupLogic::onError);

	// If the address is already an IP, we don't need a lookup.
	auto hostAddress = QHostAddress(address);
	if (!hostAddress.isNull())
	{
		setStatus(tr("Connecting to server %1:%2 (address=%3)").arg(address).arg(port).arg(hostAddress.toString()));
		_nc->connectToHost(hostAddress, port);
		return;
	}

	// Async DNS lookup.
	setStatus(tr("Resolve DNS for %1").arg(address));
	QtAsync::async([address]()
	{
		auto hostInfo = QHostInfo::fromName(address);
		return QVariant::fromValue(hostInfo);
	},
	[this, address, port](QVariant v)
	{
		auto hostInfo = v.value<QHostInfo>();
		if (hostInfo.error() != QHostInfo::NoError || hostInfo.addresses().isEmpty())
		{
			setStatusError(tr("DNS lookup failed"), hostInfo.errorString());
			return;
		}
		auto hostAddress = hostInfo.addresses().first();
		setStatus(tr("Connecting to server %1:%2 (address=%3)").arg(address).arg(port).arg(hostAddress.toString()));
		_nc->connectToHost(hostAddress, port);
	});
}

void Ts3VideoStartupLogic::authAndJoinConference()
{
	// Authenticate.
	setStatus(tr("Authenticating..."));
	QHash<QString, QVariant> authParams;
	authParams.insert("ts3_client_database_id", _args.ts3ClientDbId);
	auto reply = _nc->auth(_args.ts3Username, _joinInfo.server.password, authParams);
	QObject::connect(reply, &QCorReply::finished, [this, reply]()
	{
		HL_DEBUG(HL, QString("Auth answer: %1").arg(QString(reply->frame()->data())).toStdString());
		reply->deleteLater();

		int status;
		QString errorString;
		QJsonObject params;
		if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, errorString))
		{
			setStatusError(tr("Protocol error"), reply->frame()->data());
			return;
		}
		else if (status != 0)
		{
			setStatusError(tr("Authentication failed"), QString("%1: %2").arg(status).arg(errorString));
			return;
		}
	});

	// Wait for media socket authentication notification.
	// Do not wait longer than X seconds.. timeout.
	auto mediaAuthTimer = new QTimer(this);
	mediaAuthTimer->setSingleShot(true);
	mediaAuthTimer->start(10000);
	QObject::connect(mediaAuthTimer, &QTimer::timeout, [this]()
	{
		setStatusError(tr("Can not authorize media socket (UDP)."
						  "Make sure that all required ports are open on client and server side."
						  "You might contact your server administrator or "
						  "<a href=\"mailto:info@mfreiholz.de\">info@mfreiholz.de</a> in case of public servers."),
					   QString());
		QCorReply::autoDelete(_nc->goodbye());
		return;
	});
	QObject::connect(_nc.data(), &NetworkClient::mediaSocketAuthenticated, [this, mediaAuthTimer]()
	{
		mediaAuthTimer->stop();

		// Join channel.
		setStatus(tr("Joining conference..."));
		auto reply2 = _nc->joinChannelByIdentifier(_joinInfo.room.uid, _joinInfo.room.password);
		QObject::connect(reply2, &QCorReply::finished, [this, reply2]()
		{
			HL_DEBUG(HL, QString("Join channel answer: %1").arg(QString(reply2->frame()->data())).toStdString());
			reply2->deleteLater();

			int status;
			QString errorString;
			QJsonObject params;
			if (!JsonProtocolHelper::fromJsonResponse(reply2->frame()->data(), status, params, errorString))
			{
				setStatusError(tr("Protocol error"), reply2->frame()->data());
				return;
			}
			else if (status != 0)
			{
				setStatusError(tr("Conference join process failed"), QString("%1: %2").arg(status).arg(errorString));
				return;
			}
			this->startVideoGui();
		});
	});
}

void Ts3VideoStartupLogic::startVideoGui()
{
	// Conference settings.
	// Skip dialog with: --skip-startup-dialog
	//StartupDialog dialog(this);
	//if (!ELWS::hasArgsValue("--skip-startup-dialog"))
	//{
	//	if (dialog.exec() != QDialog::Accepted)
	//	{
	//		quitDelayed();
	//		return;
	//	}
	//}

	// Open conference video-chat UI.
	ConferenceVideoWindow::Options opts;
	ConferenceVideoWindow::loadOptionsFromConfig(opts);

	// Show video settings
	VideoSettingsDialog w(_nc, this);
	w.setModal(true);
	w.preselect(opts);
	if (w.exec() == QDialog::Accepted)
		opts = w.values();

	// Open conference window
	_window = new ConferenceVideoWindow(_nc, nullptr, 0);
	_window->applyOptions(opts);
	_window->show();

	// Close startup logic.
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
	setStatus(tr("Disconnected!"));
}

void Ts3VideoStartupLogic::onError(QAbstractSocket::SocketError socketError)
{
	setStatusError(tr("Connection problem"), _nc->socket()->errorString());
}