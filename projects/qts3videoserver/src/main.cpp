#include <QCoreApplication>
#include <QSettings>
#include <QFile>

#include "humblelogging/api.h"
#include "humblesrvproc/api.h"

#include "ts3video.h"
#include "elws.h"

#include "ts3videoserver.h"

HUMBLE_LOGGER(HL, "server");

/*!
  \return 0 = OK;
*/
int updateOptionsByArgs(TS3VideoServerOptions &opts)
{
  HL_INFO(HL, QString("Configure by arguments").toStdString());
  opts.address = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--address", opts.address.toString()).toString());
  opts.port = ELWS::getArgsValue("--port", opts.port).toUInt();
  opts.wsStatusAddress = ELWS::getQHostAddressFromString(ELWS::getArgsValue("--wsstatus-address", opts.wsStatusAddress.toString()).toString());
  opts.wsStatusPort = ELWS::getArgsValue("--wsstatus-port", opts.wsStatusPort).toUInt();
  opts.connectionLimit = ELWS::getArgsValue("--connection-limit", opts.connectionLimit).toInt();
  opts.bandwidthReadLimit = ELWS::getArgsValue("--bandwidth-read-limit", opts.bandwidthReadLimit).toULongLong();
  opts.bandwidthWriteLimit = ELWS::getArgsValue("--bandwidth-write-limit", opts.bandwidthWriteLimit).toULongLong();
  opts.validChannels.clear();
  opts.password = ELWS::getArgsValue("--password", opts.password).toString();
  return 0;
}

/*!
  \return 0 = OK;
 */
int updateOptionsByConfig(TS3VideoServerOptions &opts, const QString &filePath)
{
  HL_INFO(HL, QString("Configure by file (path=%1)").arg(filePath).toStdString());
  if (filePath.isEmpty()) {
    HL_WARN(HL, QString("No config file specified").toStdString());
    return 1;
  }
  if (!QFile::exists(filePath)) {
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
  conf.endGroup();
  return 0;
}

/*!
  Updates server options by license.
  \return 0 = OK; 1 = No valid license; 
 */
bool updateOptionsByLicense(TS3VideoServerOptions &opts, const QString &filePath)
{
  HL_INFO(HL, QString("Configure by license (file=%1)").arg(filePath).toStdString());
  return 0;
}

/*!
  Initializes the base server components.
  Note: It's required to run an event loop after this function.
 */
/*int initServer(int argc, char *argv[])
{
  auto &a = *qApp;

  // Initialize logging.
  auto& hlFactory = humble::logging::Factory::getInstance();
  hlFactory.setDefaultFormatter(new humble::logging::PatternFormatter("[%date][%lls][pid=%pid][tid=%tid] %m\n"));
  hlFactory.registerAppender(new humble::logging::ConsoleAppender());
  hlFactory.registerAppender(new humble::logging::FileAppender(std::string("ts3videoserver.log"), true));
  hlFactory.changeGlobalLogLevel(humble::logging::LogLevel::Debug);

  HL_INFO(HL, QString("Server startup (version=%1)").arg(a.applicationVersion()).toStdString());
  HL_INFO(HL, QString("Organization: %1").arg(a.organizationName()).toStdString());
  HL_INFO(HL, QString("Organization domain: %1").arg(a.organizationDomain()).toStdString());

  // Initialize server options (from ARGS).
  TS3VideoServerOptions opts;
  if (updateOptionsByArgs(opts) != 0) {
    return 1;
  }
  // Override server options by config.
  auto configFilePath = ELWS::getArgsValue("--config").toString();
  if (!configFilePath.isEmpty() && updateOptionsByConfig(opts, configFilePath) != 0) {
    return 1;
  }
  // Override server options by license.
  if (updateOptionsByLicense(opts, "FREE") != 0) {
    HL_FATAL(HL, QString("No valid license!").toStdString());
    return 7353;
  }
  
  HL_INFO(HL, QString("----- Configuration -----").toStdString());
  HL_INFO(HL, QString("Connection limit: %1").arg(opts.connectionLimit).toStdString());
  HL_INFO(HL, QString("Bandwidth read limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthReadLimit)).toStdString());
  HL_INFO(HL, QString("Bandwidth write limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthWriteLimit)).toStdString());
  HL_INFO(HL, QString("-------------------------").toStdString());

  TS3VideoServer *server(opts);
  if (!server.init())
    return 2;

  auto returnCode = a.exec();
  HL_INFO(HL, QString("Server shutdown (code=%1)").arg(returnCode).toStdString());
  return returnCode;
}*/

///////////////////////////////////////////////////////////////////////
// Server base startup logic.
///////////////////////////////////////////////////////////////////////

class AppBootstrapLogic : public QObject
{
  TS3VideoServer *_server; ///< This is the actual server (TODO: Create more than one -> Virtual Server)

public:
  AppBootstrapLogic()
    : _server(nullptr)
  {}

  int init()
  {
    auto &a = *qApp;

    // Initialize logging.
    auto& hlFactory = humble::logging::Factory::getInstance();
    hlFactory.setDefaultFormatter(new humble::logging::PatternFormatter("[%date][%lls][pid=%pid][tid=%tid] %m\n"));
    hlFactory.registerAppender(new humble::logging::ConsoleAppender());
    hlFactory.registerAppender(new humble::logging::FileAppender(qApp->applicationDirPath().toStdString() + std::string("/") + std::string("ts3videoserver.log"), true));
    hlFactory.changeGlobalLogLevel(humble::logging::LogLevel::Debug);

    HL_INFO(HL, QString("Server startup (version=%1)").arg(a.applicationVersion()).toStdString());
    HL_INFO(HL, QString("Organization: %1").arg(a.organizationName()).toStdString());
    HL_INFO(HL, QString("Organization domain: %1").arg(a.organizationDomain()).toStdString());

    // Initialize server options (from ARGS).
    TS3VideoServerOptions opts;
    if (updateOptionsByArgs(opts) != 0) {
      return 1;
    }
    // Override server options by config.
    auto configFilePath = ELWS::getArgsValue("--config").toString();
    if (!configFilePath.isEmpty() && updateOptionsByConfig(opts, configFilePath) != 0) {
      return 1;
    }
    // Override server options by license.
    if (updateOptionsByLicense(opts, "FREE") != 0) {
      HL_FATAL(HL, QString("No valid license!").toStdString());
      return 7353;
    }

    HL_INFO(HL, QString("----- Configuration -----").toStdString());
    HL_INFO(HL, QString("Connection limit: %1").arg(opts.connectionLimit).toStdString());
    HL_INFO(HL, QString("Bandwidth read limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthReadLimit)).toStdString());
    HL_INFO(HL, QString("Bandwidth write limit: %1").arg(ELWS::humanReadableBandwidth(opts.bandwidthWriteLimit)).toStdString());
    HL_INFO(HL, QString("-------------------------").toStdString());

    _server = new TS3VideoServer(opts, this);
    if (!_server->init())
      return 2;
    
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////
// Service/Daemon related code.
///////////////////////////////////////////////////////////////////////

#include <QThread>

class AppThread : public QThread
{
  int _argc;
  char **_argv;
  QAtomicInt _stopFlag;
  int _returnCode;

public:
  AppThread(int argc, char *argv[], QObject *parent) : _argc(0), _argv(0), _stopFlag(0)
  {
    _argc = argc;
    if (_argc > 0) {
      _argv = new char*[_argc];
      for (auto i = 0; i < argc; ++i) {
        _argv[i] = new char[strlen(argv[i]) + 1];
        strcpy(_argv[i], argv[i]);
      }
    }
    
  }

  ~AppThread()
  {
  }

  int returnCode() const
  {
    return _returnCode;
  }

  virtual void run()
  {
    AppBootstrapLogic logic;
    _returnCode = logic.init();
    exec();
  }
};

///////////////////////////////////////////////////////////////////////

AppThread *gAppThread = nullptr;

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

int _main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  a.setOrganizationName("insaneFactory");
  a.setOrganizationDomain("http://www.insanefactory.com/");
  a.setApplicationName("TS3 Video Server");
  a.setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);

  // Mode: Console attached.
  // Run as blocking process in main-thread.
  if (ELWS::hasArgsValue("--console")) {
    AppBootstrapLogic abl;
    if (abl.init() != 0)
      return 1;
    return a.exec();
  }

  // Mode: Daemon (Win32 Service)
  gAppThread = new AppThread(argc, argv, nullptr);

  // Initialize service configuration.
  hbl_service_config_t *conf = hbl_service_create_config();
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

#ifdef _WIN32
int main(int argc, char *argv[])
{
  return _main(argc, argv);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  return _main(__argc, __argv);
}
#endif