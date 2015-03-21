#ifdef _WIN32
#include <Windows.h>
#endif

#include "QtCore/QCoreApplication"
#include "QtCore/QFileInfo"
#include "QtCore/QDateTime"
#include "QtCore/QDir"

#include "humblelogging/api.h"

#include "qswserverlib/includes.h"

HUMBLE_LOGGER(HL, "default");

///////////////////////////////////////////////////////////////////////////////
// Helper
///////////////////////////////////////////////////////////////////////////////

QString getArgumentValue(const QStringList &args, const QString &key, const QString &defaultValue)
{
  const int pos = args.indexOf(key);
  if (pos > -1 && pos + 1 < args.size()) {
    return args.at(pos + 1);
  }
  return defaultValue;
}

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	// Initialize logging.
	humble::logging::Factory &fac = humble::logging::Factory::getInstance();
	fac.setDefaultLogLevel(humble::logging::LogLevel::Warn);
	fac.registerAppender(new humble::logging::ConsoleAppender());
	fac.registerAppender(new humble::logging::RollingFileAppender(QDir::temp().absoluteFilePath("streamwall-server.log").toStdString(), true, 5));

	// Initialize Qt application framework basics.
	QCoreApplication qapp(argc, argv);
	qapp.setApplicationName("qswserverapp");
	qapp.setOrganizationName("insaneFactory");
	qapp.setOrganizationDomain("http://www.insanefactory.com");

	// Create default configuration.
	//auto settings = SettingsService::instance().getUserSettings();
	//settings->createValue("common/enable_keep_alive_checks",      false);
	//settings->createValue("common/keep_alive_check_frequency",    10000);
	//settings->createValue("common/keep_alive_response_timeout",   20000);
	//settings->createValue("common/controlling_port",              6666);
	//settings->createValue("common/streaming_port",                6667);
	//settings->setValue("common/lastrun", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

	// Initialize crypto and random generation.
	//CryptoController::initialize();
	//qsrand(time(0));

  const QStringList arguments = qapp.arguments();
  const bool doMaster = arguments.contains("--master");
  const bool doStream = arguments.contains("--stream");
  const bool doControl = arguments.contains("--control");

  // Start MASTER server.
  /*MasterServer *masterServer = 0;
  if (doMaster) {
    MasterServer::Options opts;
    opts.mediaNodesPort = getArgumentValue(arguments, "--master-stream-port", "6660").toUInt();
    opts.clientNodesPort = getArgumentValue(arguments, "--master-control-port", "6661").toUInt();
    opts.ctrlWsPort = getArgumentValue(arguments, "--master-control-ws-port", "6662").toUInt();

    masterServer = new MasterServer();
    if (!masterServer->startup(opts)) {
      return 3;
    }
  }*/

  // Start STREAM/MEDIA server.
  /*MediaStreamingServer *mediaNode = 0;
  if (doStream) {
    MediaStreamingServer::Options mediaOpts;
    mediaOpts.streamingPort = getArgumentValue(arguments, "--stream-port", "6667").toUInt();
    mediaOpts.externalStreamingAddress = getArgumentValue(arguments, "--stream-external-address", "127.0.0.1");
    mediaOpts.externalStreamingPort = getArgumentValue(arguments, "--stream-external-port", "6667").toUInt();
    mediaOpts.masterServerAddress = getArgumentValue(arguments, "--stream-master-address", "127.0.0.1");
    mediaOpts.masterServerPort = getArgumentValue(arguments, "--stream-master-port", "6660").toUInt();

    mediaNode = new MediaStreamingServer(mediaOpts);
    if (!mediaNode->startup()) {
      return 4;
    }
  }*/

  // Start CONTROL server.
  ControlServer *controlServer = 0;
  if (doControl) {
    ControlServer::Options opts;
    opts.address = getArgumentValue(arguments, "--control-address", QString());
    opts.port = getArgumentValue(arguments, "--control-port", "6666").toUInt();
    opts.masterServerAddress = getArgumentValue(arguments, "--control-master-address", QString());
    opts.masterServerPort = getArgumentValue(arguments, "--control-master-port", "0").toUInt();

    controlServer = new ControlServer();
    if (!controlServer->startup(opts)) {
      return 5;
    }
  }

  const int code = qapp.exec();
  HL_DEBUG(HL, QString("Application exits (code=%1)").arg(code).toStdString());
  
  //CryptoController::shutdown();

  return code;
}
