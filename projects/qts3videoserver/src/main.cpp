#include <QCoreApplication>

#include "ts3videoserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  
  TS3VideoServer server(nullptr);
  if (!server.init()) {
    return 1;
  }
  return a.exec();
}
