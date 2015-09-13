#ifdef _WIN32
#include <Windows.h>
#endif

#include <QCoreApplication>
#include <QSettings>
#include <QFile>
#include <QDir>

#include "humblelogging/api.h"
#include "humblesrvproc/api.h"

#include "ts3video.h"
#include "elws.h"

#include "virtualserver.h"

HUMBLE_LOGGER(HL, "server");

/*!
    \return 0 = OK;
*/
int updateOptionsByArgs(VirtualServerOptions& opts)
{
	//HL_INFO(HL, QString("Configure by arguments").toStdString());
	opts.address = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--address", opts.address.toString()).toString());
	opts.port = ELWS::getArgsValue("--port", opts.port).toUInt();
	opts.wsStatusAddress = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--wsstatus-address", opts.wsStatusAddress.toString()).toString());
	opts.wsStatusPort = ELWS::getArgsValue("--wsstatus-port", opts.wsStatusPort).toUInt();
	opts.connectionLimit = ELWS::getArgsValue("--connection-limit", opts.connectionLimit).toInt();
	opts.bandwidthReadLimit = ELWS::getArgsValue("--bandwidth-read-limit", opts.bandwidthReadLimit).toULongLong();
	opts.bandwidthWriteLimit = ELWS::getArgsValue("--bandwidth-write-limit", opts.bandwidthWriteLimit).toULongLong();
	opts.validChannels.clear();
	opts.password = ELWS::getArgsValue("--password", opts.password).toString();
	opts.adminPassword = ELWS::getArgsValue("--admin-password", opts.adminPassword).toString();
	return 0;
}

/*!
    \return 0 = OK;
*/
int updateOptionsByConfig(VirtualServerOptions& opts, const QString& filePath)
{
	//HL_INFO(HL, QString("Configure by file (path=%1)").arg(filePath).toStdString());
	if (filePath.isEmpty())
	{
		HL_WARN(HL, QString("No config file specified").toStdString());
		return 1;
	}
	if (!QFile::exists(filePath))
	{
		HL_WARN(HL, QString("Configuration file does not exist (path=%1)").arg(filePath).toStdString());
		return 2;
	}
	QSettings conf(filePath, QSettings::IniFormat);
	conf.beginGroup("default");
	opts.address = ELWS::getQHostAddressFromString(conf.value("address", opts.address.toString()).toString());
	opts.port = conf.value("port", opts.port).toUInt();
	opts.wsStatusAddress = ELWS::getQHostAddressFromString(conf.value("wsstatus-address", opts.wsStatusAddress.toString()).toString());
	opts.wsStatusPort = conf.value("wsstatus-port", opts.wsStatusPort).toUInt();
	opts.connectionLimit = conf.value("connectionlimit", opts.connectionLimit).toInt();
	opts.bandwidthReadLimit = conf.value("bandwidthreadlimit", opts.bandwidthReadLimit).toULongLong();
	opts.bandwidthWriteLimit = conf.value("bandwidthwritelimit", opts.bandwidthWriteLimit).toULongLong();
	opts.validChannels = opts.validChannels;
	opts.password = conf.value("password", opts.password).toString();
	opts.adminPassword = conf.value("adminpassword", opts.adminPassword).toString();
	conf.endGroup();

	conf.beginGroup("teamspeak3-bridge");
	opts.ts3Enabled = conf.value("enabled", opts.ts3Enabled).toBool();
	opts.ts3Address = ELWS::getQHostAddressFromString(conf.value("address", opts.ts3Address.toString()).toString());
	opts.ts3Port = conf.value("port", opts.port).toUInt();
	opts.ts3LoginName = conf.value("loginname", opts.ts3LoginName).toString();
	opts.ts3LoginPassword = conf.value("loginpassword", opts.ts3LoginPassword).toString();
	opts.ts3Nickname = conf.value("nickname").toString();
	opts.ts3VirtualServerPort = conf.value("virtualserverport", opts.ts3VirtualServerPort).toUInt();

	auto sgl = conf.value("allowedservergroups").toStringList();
	for (auto i = 0; i < sgl.size(); ++i)
		if (!sgl[i].isEmpty())
			opts.ts3AllowedServerGroups.append(sgl[i].toULongLong());

	conf.endGroup();
	return 0;
}

/*!
    Updates server options by license.
    \return 0 = OK; 1 = No valid license;
*/
bool updateOptionsByLicense(VirtualServerOptions& opts, const QString& filePath)
{
	//HL_INFO(HL, QString("Configure by license (file=%1)").arg(filePath).toStdString());
	return 0;
}

///////////////////////////////////////////////////////////////////////
// Server base startup logic.
///////////////////////////////////////////////////////////////////////

class AppBootstrapLogic : public QObject
{
	VirtualServer* _server; ///< This is the actual server (TODO: Create more than one -> Virtual Server)

public:
	AppBootstrapLogic()
		: _server(nullptr)
	{}

	int init()
	{
		auto& a = *qApp;

		// Initialize logging.
		auto& fac = humble::logging::Factory::getInstance();
		fac.setConfiguration(new humble::logging::SimpleConfiguration(humble::logging::LogLevel::Info));
		fac.setDefaultFormatter(new humble::logging::PatternFormatter("%date\t%lls\tpid=%pid\ttid=%tid\t%m\n"));
		fac.registerAppender(new humble::logging::FileAppender(QDir::temp().filePath("ts3video-server.log").toStdString(), true));

		// Initialize server options (from ARGS).
		VirtualServerOptions opts;
		if (updateOptionsByArgs(opts) != 0)
		{
			return 1;
		}
		// Override server options by config.
		auto configFilePath = ELWS::getArgsValue("--config", a.applicationDirPath() + QString("/default.ini")).toString();
		if (!configFilePath.isEmpty() && updateOptionsByConfig(opts, configFilePath) != 0)
		{
			return 1;
		}
		// Override server options by license.
		if (updateOptionsByLicense(opts, "FREE") != 0)
		{
			HL_FATAL(HL, QString("No valid license!").toStdString());
			return 7353;
		}

		HL_INFO(HL, QString("----- Server startup ----").toStdString());
		HL_INFO(HL, QString("Version: %1").arg(a.applicationVersion()).toStdString());
		HL_INFO(HL, QString("Organization: %1").arg(a.organizationName()).toStdString());
		HL_INFO(HL, QString("Organization domain: %1").arg(a.organizationDomain()).toStdString());
		HL_INFO(HL, QString("----- Limits ------------").toStdString());
		HL_INFO(HL, QString("Connection limit: %1").arg(opts.connectionLimit).toStdString());
		HL_INFO(HL, QString("Bandwidth read limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthReadLimit)).toStdString());
		HL_INFO(HL, QString("Bandwidth write limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthWriteLimit)).toStdString());
		HL_INFO(HL, QString("-------------------------").toStdString());

		_server = new VirtualServer(opts, this);
		if (!_server->init())
		{
			return 1;
		}

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////
// Service/Daemon related code.
///////////////////////////////////////////////////////////////////////

#include <QThread>

/*! Makes it possible to run the program logic in a separate thread.
*/
class AppThread : public QThread
{
	int _argc;
	char** _argv;
	QAtomicInt _stopFlag;
	int _returnCode;

public:
	AppThread(int argc, char* argv[], QObject* parent) : _argc(0), _argv(0), _stopFlag(0)
	{
		_argc = argc;
		if (_argc > 0)
		{
			_argv = new char* [_argc];
			for (auto i = 0; i < argc; ++i)
			{
				_argv[i] = new char[strlen(argv[i]) + 1];
				strcpy(_argv[i], argv[i]);
			}
		}
	}

	~AppThread()
	{
		// TODO Clean up.
	}

	int returnCode() const
	{
		return _returnCode;
	}

	virtual void run()
	{
		AppBootstrapLogic logic;
		_returnCode = logic.init();
		if (_returnCode != 0)
			return;
		exec();
	}
};

///////////////////////////////////////////////////////////////////////

AppThread* gAppThread = nullptr;

int serviceStart()
{
	gAppThread->start();
	return 0;
}

int serviceStop()
{
	gAppThread->quit();
	gAppThread->wait();
	return 0;
}

///////////////////////////////////////////////////////////////////////
// Process entry
///////////////////////////////////////////////////////////////////////

int _main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	a.setOrganizationName("insaneFactory");
	a.setOrganizationDomain("http://www.insanefactory.com/");
	a.setApplicationName("Video Server");
	a.setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);

#ifdef _WIN32
	// Mode: Daemon (Win32 Service)
	if (ELWS::hasArgsValue("--service"))
	{
		gAppThread = new AppThread(argc, argv, nullptr);

		// Initialize service configuration.
		hbl_service_config_t* conf = hbl_service_create_config();
		conf->start = &serviceStart;
		conf->stop = &serviceStop;

		// Run service.
		int exitCode = hbl_service_run(conf);
		exitCode = gAppThread->returnCode();

		// Clean up.
		hbl_service_free_config(conf);
		delete gAppThread;
		return exitCode;
	}
#endif

	// Mode: Console attached.
	// Run as blocking process in main-thread.
	AppBootstrapLogic abl;
	auto exitCode = abl.init();
	if (exitCode != 0)
		return exitCode;

#ifdef _WIN32
	if (ELWS::hasArgsValue("--console"))
		AllocConsole();
#endif

	return a.exec();
}

/*  #include "teamspeak3/ts3serverquery.h"
    int test_serverquery(int argc, char* argv[])
    {
    (void)argc;
    (void)argv;

    TS3ServerQuery sq;

    // Write

    QList<QPair<QString, QStringList> > p;

    p.clear();
    p.append(qMakePair(QString("client_login_name"), QStringList() << "admin"));
    p.append(qMakePair(QString("client_login_password"), QStringList() << "Very Complicated Password here!"));
    qDebug() << sq.createCommand("login", p);

    p.clear();
    qDebug() << sq.createCommand("quit", p);

    p.clear();
    p.append(qMakePair(QString("client_login_name"), QStringList() << "admin"));
    qDebug() << sq.createCommand("clientlist", p);

    // Read (Parse)
    auto res = sq.parse(QByteArray("clid=5 cid=7 client_database_id=40 client_nickname=ScP client_type=0 client_away=1 client_away_message=not\\shere|clid=6 cid=7 client_database_id=41 client_nickname=Manuel\\sFreiholz client_type=0 client_away=0 client_unique_identifier=FPMPSC6MXqXq751dX7BKV0JniSo=\r\nerror id=0 msg=ok\r\n"));
    res.debugOut();
    res = sq.parse(QByteArray("key=value"));
    res = sq.parse(QByteArray("key"));
    res = sq.parse(QByteArray("client_unique_identifier=gZ7K[...]GIik="));

    return 0;
    }*/

int main(int argc, char* argv[])
{
	//return test_serverquery(argc, argv);
	return _main(argc, argv);
}

#ifdef _WIN32
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return _main(__argc, __argv);
}
#endif
