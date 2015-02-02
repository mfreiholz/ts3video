#include <QCoreApplication>

#include "humblelogging/api.h"

#include "ts3videoserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  // Initialize logging.
  auto& hlFactory = humble::logging::Factory::getInstance();
  hlFactory.registerAppender(new humble::logging::ConsoleAppender());
  
  // Startup server.
  TS3VideoServer server(nullptr);
  if (!server.init()) {
    return 1;
  }

  return a.exec();
}
