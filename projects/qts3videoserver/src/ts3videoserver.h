#ifndef TS3VIDEOSERVER_H
#define TS3VIDEOSERVER_H

#define TS3VIDEOSERVER_VERSION 1

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>

#include "qcorserver.h"

class ClientConnectionHandler;
class ClientEntity;
class ChannelEntity;

class TS3VideoServer : public QObject
{
  Q_OBJECT
  friend class ClientConnectionHandler;

public:
  TS3VideoServer(QObject *parent);
  ~TS3VideoServer();

private:
  // Listens for new client connections.
  QCorServer _corServer;

  // Information about connected clients.
  int _nextClientId;
  QHash<int, ClientEntity*> _clients; ///< Maps client-ids to their info object.
  QHash<int, ClientConnectionHandler*> _connections; ///< Maps client-ids to their connection handlers.

  // Information about existing channels.
  int _nextChannelId;
  QHash<int, ChannelEntity*> _channels; ///< Maps channel-ids to their info object.
  QHash<int, QSet<int> > _participants; ///< Maps channel-ids to client-ids.
};

// TODO Move into shared library.
#include <QJsonObject>
class ClientEntity
{
public:
  void fromQJsonObject(const QJsonObject &obj) {}
  QJsonObject toQJsonObject() const
  {
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    return obj;
  }

public:
  int id;
  QString name;
};

// TODO Move into shared library.
#include <QJsonObject>
class ChannelEntity
{
public:
  void fromQJsonObject(const QJsonObject &obj) {}
  QJsonObject toQJsonObject() const
  {
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    return obj;
  }

public:
  int id;
  QString name;
};

#endif