#ifndef ELWS_H
#define ELWS_H

#include <QString>
#include <QVariant>
#include <QRect>
#include <QPoint>
#include <QHostAddress>

class ELWS
{
public:
  static bool hasArgsValue(const QString &key);
  static QVariant getArgsValue(const QString &key, const QVariant &defaultValue = QVariant());

  static QString getUserName();

  static bool registerURISchemeHandler(const QString &scheme, const QString &title, const QString &modulePath, const QString &moduleArgs);
  static bool unregisterURISchemeHandler(const QString &scheme);

  static void calcScaledAndCenterizedImageRect(const QRect &surfaceRect, QRect &imageRect, QPoint &offset);

  static QString humanReadableSize(quint64 bytes);
  static QString humanReadableBandwidth(quint64 bytesPerSecond);

  static bool isVersionSupported(const QString &version, const QString &supportedVersions);

  static QHostAddress getQHostAddressFromString(const QString &s);
};

#endif