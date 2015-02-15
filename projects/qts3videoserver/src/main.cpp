#include <QCoreApplication>

#include "humblelogging/api.h"

#include "ts3videoserver.h"

HUMBLE_LOGGER(HL, "server");

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  a.setOrganizationName("insaneFactory");
  a.setOrganizationDomain("http://www.insanefactory.com/");
  a.setApplicationName("iF.TS3VideoServer");
  a.setApplicationVersion("1.0 ALPHA");

  // Initialize logging.
  auto& hlFactory = humble::logging::Factory::getInstance();
  hlFactory.registerAppender(new humble::logging::ConsoleAppender());
  hlFactory.registerAppender(new humble::logging::FileAppender(std::string("ts3videoserver.log"), false));
  hlFactory.changeGlobalLogLevel(humble::logging::LogLevel::Debug);
  
  HL_INFO(HL, QString("Server startup (version=%1)").arg(a.applicationVersion()).toStdString());

  // Startup server.
  TS3VideoServer server(nullptr);
  if (!server.init()) {
    return 1;
  }

  auto returnCode = a.exec();
  HL_INFO(HL, QString("Server shutdown (code=%1)").arg(returnCode).toStdString());
  return returnCode;
}
