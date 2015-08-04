#ifndef TS3SERVERQUERY_H
#define TS3SERVERQUERY_H

#include <QString>
#include <QList>
#include <QPair>
#include <QHash>
#include <QStringList>

class TS3ServerQueryResponse;

/*
  TS3ServerQuery helps with the Teamspeak Server Query console.

  Command Order:
    login ...
    select virtualserver ...

  \note This class may throw Exceptions.
*/
class TS3ServerQuery
{
public:
  TS3ServerQuery();

  QString escape(const QString& s) const;
  QString unescape(const QString& s) const;

  QString createCommand(
    const QString& cmd,
    const QList<QPair<QString, QStringList> >& parameters = QList<QPair<QString, QStringList> >(),
    const QStringList& options = QStringList()
  ) const;

  TS3ServerQueryResponse parse(const QByteArray& data);

private:
  QHash<QChar, QString> _escapes;
};


class TS3ServerQueryResponse
{
public:
  int errorCode;
  QString errorMessage;
  QList<QHash<QString, QString> > items;

  void debugOut();
};

#endif