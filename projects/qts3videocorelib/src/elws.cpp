#include "elws.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include <Lmcons.h>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

QVariant ELWS::getArgsValue(const QString &key, const QVariant &defaultValue)
{
  auto pos = qApp->arguments().indexOf(key);
  if (pos < 0 || pos + 1 >= qApp->arguments().size()) {
    return defaultValue;
  }
  return qApp->arguments().at(pos + 1);
}

QString ELWS::getUserName()
{
#ifdef Q_OS_WIN
  TCHAR username[UNLEN + 1];
  DWORD usernameLength = UNLEN + 1;
  if (GetUserName(username, &usernameLength)) {
    return QString::fromLocal8Bit((char*) username);
  }
  return QString();
#elif Q_OS_UNIX

#else
  return QString();
#endif
}

bool ELWS::registerURISchemeHandler(const QString &scheme, const QString &title, const QString &modulePath, const QString &commandArgumentLine)
{
#ifdef Q_OS_WIN
  QString moduleFilePath = modulePath;
  if (moduleFilePath.isEmpty()) {
    char lpFileName[MAX_PATH];
    if (!GetModuleFileName(NULL, lpFileName, MAX_PATH)) {
      return false;
    }
    moduleFilePath = QDir::toNativeSeparators(QFileInfo(QString::fromLocal8Bit(lpFileName)).absoluteFilePath());
  }

  QSettings registry("HKEY_CLASSES_ROOT\\" + scheme, QSettings::NativeFormat);
  registry.remove(QString());
  registry.setValue("Default", "URL:" + title.isEmpty() ? scheme : title);
  registry.setValue("URL Protocol", "");
  registry.setValue("DefaultIcon/Default", moduleFilePath);
  registry.setValue("shell/open/command/Default", QString("\"") + moduleFilePath + QString("\" ") + commandArgumentLine);
#endif
  return false;
}

bool ELWS::unregisterURISchemeHandler(const QString &scheme)
{
#ifdef Q_OS_WIN
  QSettings registry("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
  registry.remove(scheme);
#endif
  return true;
}

void ELWS::calcScaledAndCenterizedImageRect(const QRect &surfaceRect, QRect &imageRect, QPoint &offset)
{
  auto surfaceRatio = (float)surfaceRect.width() / (float)surfaceRect.height();
  auto imageRatio = (float)imageRect.width() / (float)imageRect.height();
  
  auto scaleFactor = 1.0F;
  auto x = 0, y = 0;

  if (surfaceRatio < imageRatio) {
    scaleFactor = (float)surfaceRect.height() / (float)imageRect.height();
    imageRect.setWidth((float)imageRect.width() * scaleFactor);
    imageRect.setHeight((float)imageRect.height() * scaleFactor);
    x = ((float)imageRect.width() - (float)surfaceRect.width()) / 2;
  } else {
    scaleFactor = (float)surfaceRect.width() / (float)imageRect.width();
    imageRect.setWidth((float)imageRect.width() * scaleFactor);
    imageRect.setHeight((float)imageRect.height() * scaleFactor);
    y = ((float)imageRect.height() - (float)surfaceRect.height()) / 2;
  }
  offset.setX(-x);
  offset.setY(-y);
}