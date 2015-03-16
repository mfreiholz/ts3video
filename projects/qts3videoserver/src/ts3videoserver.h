#ifndef TS3VIDEOSERVER_H
#define TS3VIDEOSERVER_H

#define TS3VIDEOSERVER_VERSION 1

#include <limits.h>

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>
#include <QHostAddress>

#include "qcorserver.h"

#include "mediasockethandler.h"
#include "websocketstatusserver.h"

class ClientConnectionHandler;
class ClientEntity;
class ChannelEntity;

/*!
 * Options to run a TS3VideoServer instance.
 */
class TS3VideoServerOptions
{
public:
  // The address and port on which the server listens for new connections.
  QHostAddress address = QHostAddress::Any;
  quint16 port = 6000;

  // The maximum number of parallel client connections.
  int connectionLimit = std::numeric_limits<int>::max();

  // The maximum bandwidth the server may use.
  // New connections will be blocked, if the server's bandwidth
  // usage reaches this value.
  double bandwidthLimit = std::numeric_limits<double>::max();

  // List of valid TS3 channel IDs users are allowed to join.
  // Leave empty for no restrictions on channel-ids.
  QList<quint64> validChannels;
};

/*!
 */
class TS3VideoServer : public QObject
{
  Q_OBJECT
  friend class ClientConnectionHandler;
  friend class WebSocketStatusServer;

public:
  TS3VideoServer(const TS3VideoServerOptions &opts, QObject *parent = nullptr);
  ~TS3VideoServer();
  bool init();

private:
  void updateMediaRecipients();
  ChannelEntity* addClientToChannel(int clientId, int channelId);
  void removeClientFromChannel(int clientId, int channelId);
  void removeClientFromChannels(int clientId);
  QList<int> getSiblingClientIds(int clientId) const;

private:
  TS3VideoServerOptions _opts;

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
  QHash<int, QSet<int> > _client2channels; ///< Maps client-ids to channel-ids.

  // Media streaming attributes.
  MediaSocketHandler *_mediaSocketHandler;
  QHash<QString, int> _tokens; ///< Maps auth-tokens to client-ids.

  // Web-socket status server.
  WebSocketStatusServer _wsStatusServer;
};

#endif