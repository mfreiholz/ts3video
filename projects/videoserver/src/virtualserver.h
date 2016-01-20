#ifndef VirtualServer_H
#define VirtualServer_H

#include <limits.h>

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>
#include <QScopedPointer>
#include <QHostAddress>
#include <QSize>

#include "qcorserver.h"

#include "videolib/src/ts3video.h"
#include "videolib/src/networkusageentity.h"
#include "videolib/src/virtualserverconfigentity.h"

#include "action/actionbase.h"
#include "virtualserveroptions.h"

class ClientConnectionHandler;
class MediaSocketHandler;
class WebSocketStatusServer;
class ServerClientEntity;
class ServerChannelEntity;

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
	QList<int> getSiblingClientIds(int clientId, bool filterByVisibilityLevel) const;

	// Ban / unban clients.
	void ban(const QHostAddress& address);
	void unban(const QHostAddress& address);
	bool isBanned(const QHostAddress& address);

private slots:
	void onNewConnection(QCorConnection* c);
	void onMediaSocketTokenAuthentication(const QString& token, const QHostAddress& address, quint16 port);
	void onMediaSocketNetworkUsageUpdated(const NetworkUsageEntity& networkUsage);

private:
	void registerAction(const ActionPtr& action);

public:
	VirtualServerOptions _opts; ///< Complete configuration for this VirtualServer instance.
	VirtualServerConfigEntity _config; ///< Config part from VirtualServerOptions, which is send to clients.

	// Listens for new client connections.
	QCorServer _corServer;
	QHash<QString, ActionPtr> _actions;

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