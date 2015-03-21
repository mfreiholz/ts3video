#ifndef _NETWORK_SERVER_HEADER_
#define _NETWORK_SERVER_HEADER_

#include "QtCore/QObject"
#include "QtCore/QList"
#include "QtCore/QSharedPointer"
#include "QtCore/QScopedPointer"
#include "QtCore/QWeakPointer"
#include "QtCore/QPointer"
#include "QtCore/QMetaType"

#include "QtNetwork/QAbstractSocket"
#include "QtNetwork/QTcpServer"

#include "shared/settings/settingsservice.h"
#include "shared/crypto/key.h"
#include "shared/network/protocol.h"
#include "shared/network/tcp_controlling_protocol.h"
#include "shared/network/clientinfo.h"
#include "shared/network/channelinfo.h"

#include "users/userdatabase.h"
#include "messages/messagedatabase.h"

class QByteArray;
class TcpSocketHandler;
class UdpSocketHandler;
class BaseModule;


struct ClientInfoData
{
	QSharedPointer<ClientInfo> info;
	QList<QWeakPointer<ChannelInfo> > channels;
	QPointer<TcpSocketHandler> socketHandler;
};


struct ChannelInfoData
{
	QSharedPointer<ChannelInfo> info;
	QList<QWeakPointer<ClientInfo> > clients;
};


/*
	Starts a streaming server with multiple virtual channels/conferences.

	\note This object is reentrant (NOT thread-safe)
*/
class StreamingServer : public QTcpServer
{
	Q_OBJECT
public:
	StreamingServer(QObject *parent = 0);
	~StreamingServer();

	bool init();

	bool listen(const QHostAddress &address = QHostAddress::Any, 
		quint16 controllingPort = Protocol::DefaultControllingPort,
		quint16 streamingPort = Protocol::DefaultStreamingPort);

	void quit();

	QSharedPointer<UserDatabase> getUserDatabase() const;
	QSharedPointer<MessageDatabase> getMessageDatabase() const;

	QSharedPointer<ChannelInfo> createChannel(const QString &name = QString(), const QString &password = QString(), bool permanent = false);
	bool joinChannel(Protocol::client_id_t clientId, Protocol::channel_id_t channelId);
	bool leaveChannel(Protocol::client_id_t clientId, Protocol::channel_id_t channelId);
	bool leaveAllChannels(Protocol::client_id_t clientId);
	bool proveChannelAuthentication(Protocol::channel_id_t channelId, const QByteArray &password);

	QSharedPointer<ClientInfo>  findClientById(Protocol::client_id_t id) const;
	QSharedPointer<ClientInfo>  findClientByFingerprint(const QByteArray &fingerprint) const;
	QSharedPointer<ChannelInfo> findChannelById(Protocol::channel_id_t id) const;
	TcpSocketHandler*           findSocket(Protocol::client_id_t id) const;
	TcpSocketHandler*           findSocket(const QByteArray &fingerprint) const;
	TcpSocketHandler*           findSocket(QSharedPointer<ClientInfo> clientInfo) const;

	QString                     generateUdpAuthToken(QSharedPointer<ClientInfo> client);
	QSharedPointer<ClientInfo>  getClientForUdpAuthToken(const QString &token) const;

	QList<QSharedPointer<ChannelInfo> > getChannelsOfClient(Protocol::client_id_t id);
	QList<QSharedPointer<ClientInfo> > getClientsOfChannel(Protocol::channel_id_t id);
	QList<QSharedPointer<ClientInfo> > getSiblingClients(Protocol::client_id_t id) const;

	void updateReceiverList();

protected:
	void incomingConnection(qintptr descriptor);

private:
	bool initUserDatabase();
	bool initMessageDatabase();

private slots:
	void onUdpTokenAuthorization(const QString &token, const QHostAddress &sender, quint16 senderPort);

public:
	friend class TcpSocketHandler;
	friend class UdpSocketHandler;

	SettingsRef _config;
	QList<QSharedPointer<BaseModule> > _modules;
	QSharedPointer<UserDatabase> _userDatabase;
	QSharedPointer<MessageDatabase> _messageDatabase;
	Key _key; // OpenSSL key to enrypt/decrypt network traffic.
	QHash<Protocol::client_id_t, ClientInfoData> _clients;
	QHash<Protocol::channel_id_t, ChannelInfoData> _channels;
	QHash<QByteArray, QWeakPointer<ClientInfo> > _fingerprint2client;

	// Streaming socket
	// One socket for all clients
	QPointer<UdpSocketHandler> _udpSocket;
	QHash<QString, QWeakPointer<ClientInfo> > _udpAuthTokens;
};

#endif
