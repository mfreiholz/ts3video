#include <QCoreApplication>

#include "humblelogging/api.h"

#include "ts3video.h"
#include "elws.h"

#include "ts3videoserver.h"

HUMBLE_LOGGER(HL, "server");

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  a.setOrganizationName("insaneFactory");
  a.setOrganizationDomain("http://www.insanefactory.com/");
  a.setApplicationName("TS3 Video Server");
  a.setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);

  // Initialize logging.
  auto& hlFactory = humble::logging::Factory::getInstance();
  hlFactory.registerAppender(new humble::logging::ConsoleAppender());
  hlFactory.registerAppender(new humble::logging::FileAppender(std::string("ts3videoserver.log"), true));
  hlFactory.changeGlobalLogLevel(humble::logging::LogLevel::Debug);

  // Initialize server options (from ARGS).
  TS3VideoServerOptions opts;
  opts.port = ELWS::getArgsValue("--server-port", opts.port).toUInt();
  opts.connectionLimit = ELWS::getArgsValue("--connection-limit", opts.connectionLimit).toInt();
  opts.bandwidthReadLimit = ELWS::getArgsValue("--bandwidth-read-limit", opts.bandwidthReadLimit).toDouble();
  opts.bandwidthWriteLimit = ELWS::getArgsValue("--bandwidth-write-limit", opts.bandwidthWriteLimit).toDouble();

  // TODO Override server options by license.
  // ...
  
  HL_INFO(HL, QString("Server startup (version=%1)").arg(a.applicationVersion()).toStdString());
  HL_INFO(HL, QString("Connection limit: %1").arg(opts.connectionLimit).toStdString());
  HL_INFO(HL, QString("Bandwidth read limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthReadLimit)).toStdString());
  HL_INFO(HL, QString("Bandwidth write limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthWriteLimit)).toStdString());

  // Startup server.
  TS3VideoServer server(opts);
  if (!server.init()) {
    return 1;
  }

  auto returnCode = a.exec();
  HL_INFO(HL, QString("Server shutdown (code=%1)").arg(returnCode).toStdString());
  return returnCode;
}
