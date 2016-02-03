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

#include "qcorlib/qcorserver.h"

#include "baselib/defines.h"

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

	ServerChannelEntity* createChannel(const QString& ident = QString());
	ServerChannelEntity* addClientToChannel(ocs::clientid_t clientId, ocs::channelid_t channelId);
	void removeClientFromChannel(ocs::clientid_t clientId, ocs::channelid_t channelId);
	void removeClientFromChannels(ocs::clientid_t clientId);
	QList<ocs::clientid_t> getSiblingClientIds(ocs::clientid_t clientId, bool filterByVisibilityLevel) const;

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
	ocs::clientid_t _nextClientId;
	QHash<ocs::clientid_t, ServerClientEntity*> _clients; ///< Maps client-ids to their info object.
	QHash<ocs::clientid_t, ClientConnectionHandler*> _connections; ///< Maps client-ids to their connection handlers.

	// Information about existing conferences.
	ocs::channelid_t _nextChannelId;
	QHash<ocs::channelid_t, ServerChannelEntity*> _channels;            // Maps channel-ids to their info object.
	QHash<QString, ocs::channelid_t> _ident2channel;                    // Maps and identifier to it's matching channel-id. Optional: Only TS3VIDEO channels use this by now.
	QHash<ocs::channelid_t, QSet<ocs::clientid_t> > _participants;      // Maps channel-ids to client-ids.
	QHash<ocs::clientid_t, QSet<ocs::channelid_t> > _client2channels;   // Maps client-ids to channel-ids.
	QHash<ocs::clientid_t, QSet<ocs::clientid_t> > _sender2receiver;    // Maps sender-ids to receiver-ids (In addition to conference based mappings!)
	

	// Media streaming attributes.
	MediaSocketHandler* _mediaSocketHandler;
	QHash<QString, ocs::clientid_t> _tokens; ///< Maps auth-tokens to client-ids.

	// Web-socket status server.
	WebSocketStatusServer* _wsStatusServer;

	// Network usages (COR, Media, WebSocket, ...)
	NetworkUsageEntity _networkUsageMediaSocket;

	// Statistics database

};

#endif