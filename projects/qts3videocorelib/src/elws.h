#ifndef ELWS_H
#define ELWS_H

#include <QString>
#include <QVariant>
#include <QRect>
#include <QPoint>

class ELWS
{
public:
  static QVariant getArgsValue(const QString &key, const QVariant &defaultValue = QVariant());
  static QString getUserName();

  static bool registerURISchemeHandler(const QString &scheme, const QString &title, const QString &modulePath, const QString &moduleArgs);
  static bool unregisterURISchemeHandler(const QString &scheme);

  static void calcScaledAndCenterizedImageRect(const QRect &surfaceRect, QRect &imageRect, QPoint &offset);
};

#endif