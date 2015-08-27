#ifndef VirtualServer_H
#define VirtualServer_H

#include <limits.h>

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>
#include <QScopedPointer>
#include <QHostAddress>

#include "qcorserver.h"

#include "ts3video.h"

#include "mediasockethandler.h"
#include "websocketstatusserver.h"

class ClientConnectionHandler;
class ServerClientEntity;
class ServerChannelEntity;

/*!
    Options to run a VirtualServer instance.
*/
class VirtualServerOptions
{
public:
	// The address and port on which the server listens for new connections.
	QHostAddress address = QHostAddress::Any;
	quint16 port = IFVS_SERVER_CONNECTION_PORT;

	// The address and port of server's status and control WebSocket.
	QHostAddress wsStatusAddress = QHostAddress::LocalHost;
	quint16 wsStatusPort = IFVS_SERVER_WSSTATUS_PORT;

	// The maximum number of parallel client connections.
	int connectionLimit = std::numeric_limits<int>::max();

	// The maximum bandwidth the server may use.
	// New connections will be blocked, if the server's bandwidth
	// usage reaches this value.
	unsigned long long bandwidthReadLimit = std::numeric_limits<unsigned long long>::max();
	unsigned long long bandwidthWriteLimit = std::numeric_limits<unsigned long long>::max();

	// List of valid channel IDs users are allowed to join.
	// Leave empty for no restrictions on channel-ids.
	QList<quint64> validChannels;

	// Basic server password.
	QString password;

	// Administrator password.
	QString adminPassword;

	// TeamSpeak 3 Server Bridge.
	bool ts3Enabled = false;
	QHostAddress ts3Address = QHostAddress::LocalHost;
	quint16 ts3Port = 10011;
	QString ts3LoginName;
	QString ts3LoginPassword;
	QString ts3Nickname;
	quint16 ts3VirtualServerPort = 9987;
	QList<quint64> ts3AllowedServerGroups;
};

/*!
    TODO: Move every "private" member into the private-impl!
*/
class VirtualServerPrivate;
class VirtualServer : public QObject
{
	Q_OBJECT
	VirtualServer(const VirtualServer& other);
	VirtualServer& operator=(const VirtualServer& other);

public:
	VirtualServer(const VirtualServerOptions& opts, QObject* parent = nullptr);
	virtual ~VirtualServer();
	bool init();
	const VirtualServerOptions& options() const;
	void updateMediaRecipients();
	ServerChannelEntity* addClientToChannel(int clientId, int channelId);
	void removeClientFromChannel(int clientId, int channelId);
	void removeClientFromChannels(int clientId);
	QList<int> getSiblingClientIds(int clientId) const;

	// Ban / unban clients.
	void ban(const QHostAddress& address);
	void unban(const QHostAddress& address);
	bool isBanned(const QHostAddress& address);

private slots:
	void onNewConnection(QCorConnection* c);

public:
	QScopedPointer<VirtualServerPrivate> d;
	VirtualServerOptions _opts;

	// Listens for new client connections.
	QCorServer _corServer;

	// Information about connected clients.
	int _nextClientId;
	QHash<int, ServerClientEntity*> _clients; ///< Maps client-ids to their info object.
	QHash<int, ClientConnectionHandler*> _connections; ///< Maps client-ids to their connection handlers.

	// Information about existing channels.
	int _nextChannelId;
	QHash<int, ServerChannelEntity*> _channels; ///< Maps channel-ids to their info object.
	QHash<int, QSet<int> > _participants; ///< Maps channel-ids to client-ids.
	QHash<int, QSet<int> > _client2channels; ///< Maps client-ids to channel-ids.

	// Media streaming attributes.
	MediaSocketHandler* _mediaSocketHandler;
	QHash<QString, int> _tokens; ///< Maps auth-tokens to client-ids.

	// Web-socket status server.
	WebSocketStatusServer* _wsStatusServer;

	// Network usages (COR, Media, WebSocket, ...)
	NetworkUsageEntity _networkUsageMediaSocket;
};

#endif