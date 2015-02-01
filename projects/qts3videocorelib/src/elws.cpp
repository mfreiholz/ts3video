#include "elws.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include <Lmcons.h>
#endif

#include <QCoreApplication>

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