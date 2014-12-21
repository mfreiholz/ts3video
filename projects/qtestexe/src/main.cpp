#include <QCoreApplication>
#include <QTimer>
#include <QHostAddress>
#include "mytestobject.h"
#include "qtestclient.h"

QString getArgument(const QStringList &arguments, const QString &key, const QString &defaultValue)
{
  const int pos = arguments.indexOf(key);
  if (pos < 0) {
    return defaultValue;
  }
  return arguments.at(pos + 1);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    const QStringList args = a.arguments();
    MyTestObject obj(0);

    if (args.contains("--server")) {
      obj.startServer(QHostAddress::Any, getArgument(args, "--server-port", "5005").toUInt());
    }

    if (args.contains("--client")) {
      obj.startClient(QHostAddress(getArgument(args, "--address", "127.0.0.1")), getArgument(args, "--port", "5005").toUInt());
    }

    return a.exec();
}