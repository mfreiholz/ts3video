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

  \note This class may throw Exceptions.
*/
class TS3ServerQuery
{
public:
  TS3ServerQuery();

  QString escape(const QString& s) const;
  QString unescape(const QString& s) const;

  /*
    Builds a complete command string, which can be used on server query console.
    A command looks like:
      [command] [parameters] [options]
      login client_login_name=admin client_login_password=secret -opt1 -opt2
  */
  QString createCommand(
    const QString& cmd,
    const QList<QPair<QString, QStringList> >& parameters = QList<QPair<QString, QStringList> >(),
    const QStringList& options = QStringList()
  ) const;

  /*
    Parses data and converts it into an object.

    @param data
      e.g.: 'key1=val1 key2=val2|key1=val3 key2=val4534'

    @return Array with entity objects:
      [
        { "key1": "val1", "key2": "val2" },
        { "key1": "val3", "key2": "val4534" }
      ]
  */
  QList<QHash<QString, QString> > parseItemList(const QString& data) const;

  /*
    @param data
      e.g.: 'key1=val1 key2=val2'

    @return Entity object:
      { "key1": "val1", "key2": "val2" }
  */
  QHash<QString, QString> parseItem(const QString& data) const;

  /*
    @param data
      e.g.: 'error id=0 msg=ok'

    @return Entity with error information, code and message.
  */
  QPair<int, QString> parseError(const QString& data) const;

  /*
    Checks whether the given data matches the response error line.
  */
  bool isErrorLine(const QString& data) const;

  TS3ServerQueryResponse parse(const QByteArray& data);
  TS3ServerQueryResponse parseNext(QIODevice* device) const;

  /*
    Finds the first Item with a matching key<=>value combination.

    @param itemList
      List of multiple items.
    @param key
      The name of the key, e.g.: "cldbid"
    @param value
      The value of the ..

    @return The found Item or an empty hash.
  */
  static QHash<QString, QString> findItem(const QList<QHash<QString, QString> >& itemList, const QString& key, const QString &value);

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