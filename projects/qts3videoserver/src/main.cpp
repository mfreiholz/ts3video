#include <QCoreApplication>

#include "ts3videoserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  
  TS3VideoServer server(nullptr);

  return a.exec();
}
