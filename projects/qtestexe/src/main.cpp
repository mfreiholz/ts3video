#if defined(_WIN32)
#include <Windows.h>
#endif
#include <QCoreApplication>
#include <QTimer>
#include <QHostAddress>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include "mytestobject.h"
#include "qtestclient.h"

QString getArgument(const QStringList &arguments, const QString &key, const QString &defaultValue = QString())
{
  const int pos = arguments.indexOf(key);
  if (pos < 0) {
    return defaultValue;
  }
  return arguments.at(pos + 1);
}

void registerURISchemeHandler()
{
#if defined(_WIN32)
  char lpFileName[MAX_PATH];
  if (GetModuleFileName(NULL, lpFileName, MAX_PATH)) {
    const QString filePath = QDir::toNativeSeparators(QFileInfo(QString::fromLocal8Bit(lpFileName)).absoluteFilePath());
    QSettings registry("HKEY_CLASSES_ROOT\\streamwall", QSettings::NativeFormat);
    registry.remove(QString());
    registry.setValue("Default", "URL:StreamWall");
    registry.setValue("URL Protocol", "");
    registry.setValue("DefaultIcon/Default", filePath);
    registry.setValue("shell/open/command/Default", QString("\"") + filePath + QString("\" --client --uri \"%1\""));
  }
#endif
}

void unregisterURISchemeHandler()
{
#if defined(_WIN32)
  QSettings registry("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
  registry.remove("streamwall");
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    const QStringList args = a.arguments();
    
    //unregisterURISchemeHandler();
    //registerURISchemeHandler();

    MyTestObject obj(0);
    if (args.contains("--server")) {
      obj.startServer(QHostAddress::Any, getArgument(args, "--server-port", "5005").toUInt());
    }

    if (args.contains("--client")) {
      QHostAddress address = QHostAddress(getArgument(args, "--address", "127.0.0.1"));
      quint16 port = getArgument(args, "--port", "5005").toUInt();

      QUrl url(getArgument(args, "--uri"));
      if (url.isValid()) {
        address = QHostAddress(url.host());
        port = url.port(port);
      }

      obj.startClient(address, port);
    }

    return a.exec();
}