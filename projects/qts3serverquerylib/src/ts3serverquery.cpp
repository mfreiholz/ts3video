#include "ts3serverquery.h"
#include <QTextStream>
#include <QDebug>
#include <exception>

///////////////////////////////////////////////////////////////////////
// TS3ServerQuery
///////////////////////////////////////////////////////////////////////


/*
  Notes:
    It would be faster to use a switch() of all signs instead of using a lookup hash.

  Docs:
    http://media.teamspeak.com/ts3_literature/TeamSpeak%203%20Server%20Query%20Manual.pdf
*/
TS3ServerQuery::TS3ServerQuery()
{
  // Normal characters.
  _escapes['\\'] = "\\\\"; // ASCII -> 92
  _escapes['/'] = "\\/"; // ASCII -> 47
  _escapes[' '] = "\\s"; // ASCII -> 32
  _escapes['|'] = "\\p"; // ASCII -> 124

  // Control characters.
  _escapes['\a'] = "\a"; // ASCII -> 7
  _escapes['\b'] = "\b"; // ASCII -> 8
  _escapes['\f'] = "\f"; // ASCII -> 12
  _escapes['\n'] = "\n"; // ASCII -> 10
  _escapes['\r'] = "\r"; // ASCII -> 13
  _escapes['\t'] = "\t"; // ASCII -> 9
  _escapes['\v'] = "\v"; // ASCII -> 11
}


QString TS3ServerQuery::escape(const QString& s) const
{
  QString enc;
  for (auto i = 0, l = s.size(); i < l; ++i)
  {
    const auto& c = s[i];
    enc.append(_escapes.value(c, c));
  }
  return enc;
}


QString TS3ServerQuery::unescape(const QString& s) const
{
  // TODO Not yet implemented.
  return s;
}


QString TS3ServerQuery::createCommand(const QString& cmd, const QList<QPair<QString, QStringList> >& parameters, const QStringList& options) const
{
  if (cmd.isEmpty())
    throw new std::exception("Empty command value not allowed.");

  QString s;
  s.append(cmd);

  if (parameters.size() > 0)
  {
    for (auto i = 0; i < parameters.size(); ++i)
    {
      const auto& key = parameters[i].first;
      const auto& values = parameters[i].second;
      s.append(' ');
      for (auto j = 0; j < values.size(); ++j)
      {
        if (j > 0)
          s.append('|');
        s.append(key);
        s.append('=');
        s.append(escape(values[j]));
      }
    }
  }

  if (options.size() > 0)
  {
    for (auto i = 0; i < options.size(); ++i)
    {
      s.append(' ');
      if (!options[i].startsWith('-'))
        s.append('-');
      s.append(options[i]);
    }
  }

  s.append('\n');

  return s;
}


QList<QHash<QString, QString> > TS3ServerQuery::parseItemList(const QString& data) const
{
  QList<QHash<QString, QString> > l;
  auto items = data.split('|');
  foreach (auto itemData, items)
  {
    l.append(parseItem(itemData));
  }
  return l;
}


QHash<QString, QString> TS3ServerQuery::parseItem(const QString& data) const
{
  QHash<QString, QString> obj;
  auto kvs = data.split(' ');
  foreach (auto& kv, kvs)
  {
    auto pos = kv.indexOf('=');
    if (pos == -1)
    {
      obj[kv] = QString();
    }
    else if (pos > 0)
    {
      if (pos == kv.length() - 1)
        obj[kv.mid(0, pos)] = QString();
      else
        obj[kv.mid(0, pos)] = kv.mid(pos + 1);
    }
    else
    {
      throw new std::exception("Invalid key=value format");
    }
  }
  return obj;
}


QPair<int, QString> TS3ServerQuery::parseError(const QString& data) const
{
  QRegExp rx("error id=(.*)? msg=(.*)?");
  if (!rx.exactMatch(data))
  {
    throw new std::exception("Invalid format for error line.");
  }
  return qMakePair(rx.cap(1).toInt(), rx.cap(2));
}


bool TS3ServerQuery::isErrorLine(const QString& data) const
{
  QRegExp rx("error id=(.*)? msg=(.*)?");
  return rx.exactMatch(data);
}


TS3ServerQueryResponse TS3ServerQuery::parse(const QByteArray& data)
{
  QRegExp rx("error id=(.*)? msg=(.*)?");

  TS3ServerQueryResponse res;
  bool isEvent = false;
  if (isEvent)
  {
    throw new std::exception("Event from server: Not yet implemented!");
  }
  else
  {
    QTextStream in(data, QIODevice::ReadOnly);
    in.setCodec("UTF-8");
    QString line;
    while (!(line = in.readLine()).isEmpty())
    {
      qDebug() << line;
      if (rx.exactMatch(line))
      {
        res.errorCode = rx.cap(1).toInt();
        res.errorMessage = rx.cap(2);
        break;
      }
      res.items = parseItemList(line);
    }
  }
  return res;
}


TS3ServerQueryResponse TS3ServerQuery::parseNext(QIODevice* device) const
{
  QTextStream in(device);
  in.setCodec("UTF-8");

  QString line;


  return TS3ServerQueryResponse();
}


void TS3ServerQueryResponse::debugOut()
{
  for (auto i = 0; i < items.size(); ++i)
  {
    qDebug() << QString("Element %1").arg(i);
    QHashIterator<QString, QString> itr(items[i]);
    while (itr.hasNext())
    {
      itr.next();
      qDebug() << QString("%1 = %2").arg(itr.key()).arg(itr.value());
    }
  }
}