#include <QCoreApplication>
#include <QSettings>
#include <QFile>

#include "humblelogging/api.h"

#include "ts3video.h"
#include "elws.h"

#include "ts3videoserver.h"

HUMBLE_LOGGER(HL, "server");

/*!
  \return 0 = OK;
 */
int updateOptionsByConfig(TS3VideoServerOptions &opts, const QString &filePath)
{
  if (filePath.isEmpty()) {
    HL_WARN(HL, QString("No config file specified").toStdString());
    return 1;
  }
  if (!QFile::exists(filePath)) {
    HL_WARN(HL, QString("Config file does not exist (path=%1)").arg(filePath).toStdString());
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
  conf.endGroup();
  return 0;
}

/*!
  Updates server options by license.
  \return 0 = OK; 1 = No valid license; 
 */
bool updateOptionsByLicense(TS3VideoServerOptions &opts)
{
  // Load settings from license.
  return 0;
}

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
  opts.address = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--address", opts.address.toString()).toString());
  opts.port = ELWS::getArgsValue("--port", opts.port).toUInt();
  opts.wsStatusAddress = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--wsstatus-address", opts.wsStatusAddress.toString()).toString());
  opts.wsStatusPort = ELWS::getArgsValue("--wsstatus-port", opts.wsStatusPort).toUInt();
  opts.connectionLimit = ELWS::getArgsValue("--connection-limit", opts.connectionLimit).toInt();
  opts.bandwidthReadLimit = ELWS::getArgsValue("--bandwidth-read-limit", opts.bandwidthReadLimit).toULongLong();
  opts.bandwidthWriteLimit = ELWS::getArgsValue("--bandwidth-write-limit", opts.bandwidthWriteLimit).toULongLong();
  opts.validChannels.clear();
  opts.password = ELWS::getArgsValue("--password", opts.password).toString();

  // Override server options by config.
  auto configFilePath = ELWS::getArgsValue("--config").toString();
  if (!configFilePath.isEmpty() && updateOptionsByConfig(opts, configFilePath) != 0) {
    return 1;
  }

  // Override server options by license.
  if (updateOptionsByLicense(opts) != 0) {
    HL_FATAL(HL, QString("No valid license!").toStdString());
    return 7353;
  }
  
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
