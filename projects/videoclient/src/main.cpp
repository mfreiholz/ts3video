#include <QObject>
#include <QTimer>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QCamera>
#include <QCameraInfo>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QDir>

#include "humblelogging/api.h"

#include "qcorreply.h"

#include "ts3video.h"
#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"

#include "startuplogic.h"
#include "startup/ts3videostartuplogic.h"

#include "cameraframegrabber.h"
#include "videowidget.h"
#include "videocollectionwidget.h"
#include "networkclient/networkclient.h"
#include "clientapplogic.h"
#include "startupwidget.h"
#include "tileviewwidget.h"

HUMBLE_LOGGER(HL, "client");

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
/*int runClientAppLogic(QApplication& a)
{
	a.setOrganizationName("insaneFactory");
	a.setOrganizationDomain("http://ts3video.insanefactory.com/");
	a.setApplicationName("Video Client");
	a.setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);
	a.setQuitOnLastWindowClosed(true);

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
	StartupDialog dialog(nullptr);
	dialog.setValues(opts);
	if (!ELWS::hasArgsValue("--skip-startup-dialog"))
	{
		if (dialog.exec() != QDialog::Accepted)
		{
			QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
			return a.exec();
		}
	}
	opts = dialog.values();

	HL_INFO(HL, QString("---------------------------").toStdString());
	HL_INFO(HL, QString("Client startup (version=%1)").arg(a.applicationVersion()).toStdString());
	HL_INFO(HL, QString("Address: %1").arg(opts.serverAddress).toStdString());
	HL_INFO(HL, QString("Port: %1").arg(opts.serverPort).toStdString());
	HL_INFO(HL, QString("Username: %1").arg(opts.username).toStdString());
	HL_INFO(HL, QString("Channel ID: %1").arg(opts.channelId).toStdString());
	HL_INFO(HL, QString("Channel Ident: %1").arg(opts.channelIdentifier).toStdString());
	HL_INFO(HL, QString("Camera device ID: %1").arg(opts.cameraDeviceId).toStdString());

	ClientAppLogic win(opts, nullptr, 0);
	win.resize(600, 400);
	win.show();
	win.initNetwork();

	auto returnCode = a.exec();
	HL_INFO(HL, QString("Client shutdown (code=%1)").arg(returnCode).toStdString());
	return returnCode;
}*/

/*
	Runs a implementation of AbstractStartupLogic based on command line
	parameter "--mode".
*/
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QScopedPointer<AbstractStartupLogic> sl;
	const auto mode = ELWS::getArgsValue("--mode",QString("ts3video")).toString();
	if (mode.compare("ts3video", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new Ts3VideoStartupLogic(&a));
	}
	if (!sl)
	{
		return 500; // No startup logic.
	}
	auto returnCode = sl->exec();
	HL_INFO(HL, QString("Application exit (code=%1)").arg(returnCode).toStdString());
	return returnCode;

/*	// Initialize logging.
	auto& fac = humble::logging::Factory::getInstance();
	fac.setDefaultFormatter(new humble::logging::PatternFormatter("[%date][%lls][pid=%pid][tid=%tid] %m\n"));
	fac.registerAppender(new humble::logging::FileAppender(QDir::temp().filePath("ts3video-client.log").toStdString(), true));
	fac.changeGlobalLogLevel(humble::logging::LogLevel::Debug);

	// Show console window?
	if (ELWS::hasArgsValue("--console"))
	{
#ifdef _WIN32
		AllocConsole();
#endif
	}

	// Load stylesheet.
	if (!ELWS::hasArgsValue("--no-style"))
	{
		QFile f(":/default.css");
		f.open(QFile::ReadOnly);
		a.setStyleSheet(QString(f.readAll()));
		f.close();
	}

	return runClientAppLogic(a);*/
}