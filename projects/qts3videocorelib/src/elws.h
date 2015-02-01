#ifndef ELWS_H
#define ELWS_H

#include <QString>
#include <QVariant>

class ELWS
{
public:
  static QVariant getArgsValue(const QString &key, const QVariant &defaultValue = QVariant());
  static QString getUserName();
};

#endif