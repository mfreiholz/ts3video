#ifndef TS3VIDEOSERVER_H
#define TS3VIDEOSERVER_H

#define TS3VIDEOSERVER_VERSION 1

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>

#include "qcorserver.h"

#include "mediasockethandler.h"

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
  void updateMediaRecipients();

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

  // Media streaming attributes.
  MediaSocketHandler *_mediaSocketHandler;
  QHash<QString, int> _tokens; ///< Maps auth-tokens to client-ids.
};

#endif