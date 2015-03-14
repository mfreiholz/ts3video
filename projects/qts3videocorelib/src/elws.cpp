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
#if defined(Q_OS_WIN)
  TCHAR username[UNLEN + 1];
  DWORD usernameLength = UNLEN + 1;
  if (GetUserName(username, &usernameLength)) {
    return QString::fromLocal8Bit((char*) username);
  }
  return QString();
#elif defined(Q_OS_UNIX)

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
  offset.setX(x);
  offset.setY(y);
}

QString ELWS::humanReadableSize(quint64 bytes)
{
  // Geopbyte   (>= 1267650600228229401496703205376)
  // Brontobyte (>= 1237940039285380274899124224)
  // Yottabyte  (>= 1208925819614629174706176)
  // Zettabyte  (>= 1180591620717411303424)
  // Exabyte
  if (bytes >= 1152921504606846976) {
    auto eb = bytes / 1152921504606846976.0;
    return QString("%1 EB").arg(eb, 0, 'f', 2);
  }
  // Petabyte
  else if (bytes >= 1125899906842624) {
    auto pb = bytes / 1125899906842624.0;
    return QString("%1 PB").arg(pb, 0, 'f', 2);
  }
  // Terabyte
  else if (bytes >= 1099511627776) {
    auto tb = bytes / 1099511627776.0;
    return QString("%1 TB").arg(tb, 0, 'f', 2);
  }
  // Gigabyte
  else if (bytes >= 1073741824) {
    auto gb = bytes / 1073741824.0;
    return QString("%1 GB").arg(gb, 0, 'f', 2);
  }
  // Megabyte
  else if (bytes >= 1048576) {
    auto mb = bytes / 1048576.0;
    return QString("%1 MB").arg(mb, 0, 'f', 2);
  }
  // Kilobyte
  else if (bytes >= 1024) {
    auto kb = bytes / 1024.0;
    return QString("%1 KB").arg(kb, 0, 'f', 2);
  }
  if (bytes > 1)
    return QString("%1 Bytes").arg(bytes);
  return QString("%1 Byte").arg(bytes);
}

QString ELWS::humanReadableBandwidth(quint64 bytesPerSecond)
{
  return humanReadableSize(bytesPerSecond) + QString("/s");
}