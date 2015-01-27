#include "elws.h"

#include <QCoreApplication>

QVariant ELWS::getArgsValue(const QString &key, const QVariant &defaultValue)
{
  auto pos = qApp->arguments().indexOf(key);
  if (pos < 0 || pos + 1 >= qApp->arguments().size()) {
    return defaultValue;
  }
  return qApp->arguments().at(pos + 1);
}