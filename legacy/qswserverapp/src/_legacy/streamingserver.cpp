#include "_legacy/streamingserver.h"

#include <algorithm>

#include "QtCore/QString"
#include "QtCore/QList"
#include "QtCore/QDateTime"
#include "QtCore/QDir"
#include "QtCore/QFileInfo"
#include "QtCore/QMutableListIterator"

#include "humblelogging/api.h"

#include "shared/network/clientinfo.h"
#include "shared/network/channelinfo.h"

#include "_legacy/tcp_controlling_server_socket_handler.h"
#include "_legacy/udp_streaming_server_socket_handler.h"
#include "_legacy/modules/basemodule.h"
#include "_legacy/modules/conferencemodule.h"
#include "_legacy/modules/encryptedstoredmessagemodule.h"
#include "_legacy/modules/profilemodule.h"

#include "_legacy/users/sqlite/sqliteuserdatabase.h"
#include "_legacy/messages/sqlite/sqlitemessagedatabase.h"

HUMBLE_LOGGER(HL, "default");

///////////////////////////////////////////////////////////////////////////////
// The StreamingServer class
///////////////////////////////////////////////////////////////////////////////

StreamingServer::StreamingServer(QObject *parent)
	: QTcpServer(parent)
{
	qRegisterMetaType<QHostAddress>("QHostAddress");
	qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
	qRegisterMetaType<QSharedPointer<ClientInfo> >("QSharedPointer<ClientInfo>");
	qRegisterMetaType<QSharedPointer<ChannelInfo> >("QSharedPointer<ChannelInfo>");
}

StreamingServer::~StreamingServer()
{
	quit();
}

bool StreamingServer::init()
{
	// Load config.
	_config = SettingsService::instance().getUserSettings();

	// Create default channel (ID = 1).
	createChannel("Home (Audio & Video)", QString(), true);
	createChannel("Audio only", QString(), true)->videoEnabled = false;
	createChannel("Video only", QString(), true)->audioEnabled = false;
	createChannel("Room #1 (Password=room2)", "room2", true);
	createChannel("Room #2 (Password=room3)", "room3", true);

	// Register modules.
	_modules.append(QSharedPointer<BaseModule>(new ConferenceModule(this)));
	_modules.append(QSharedPointer<BaseModule>(new EncryptedStoredMessageModule(this)));
	_modules.append(QSharedPointer<BaseModule>(new ProfileModule(this)));

	// Initialize modules.
	for (int i = 0; i < _modules.size(); ++i) {
		if (!_modules[i]->initialize()) {
			return false;
		}
	}

	// DATABASE
	// Open database connection.
	if (!initUserDatabase()) {
		return false;
	}
	if (!initMessageDatabase()) {
		return false;
	}

	// OPEN SSH
	// Load OpenSSH key to encrypt/decrypt network traffic.
	const QString privateKey = _config->value("common/privatekey").toString();
	if (privateKey.isEmpty()) {
		HL_ERROR(HL, "No private key found in config (common/privatekey)");
		return false;
	} else if (!_key.importPrivateKey(privateKey)) {
		HL_ERROR(HL, "Invalid private key");
		return false;
	}
	return true;
}

bool StreamingServer::listen(const QHostAddress &address, quint16 controllingPort, quint16 streamingPort)
{
	// Controlling
	if (!QTcpServer::listen(address, controllingPort)) {
		HL_ERROR(HL, QString("Could not open socket for listening (protocol=TCP; address=%1; port=%3; error=%4)")
		.arg(address.toString()).arg(controllingPort).arg(errorString()).toStdString());
		return false;
	}
	HL_DEBUG(HL, QString("Bound socket (protocol=TCP; address=%1; port=%2)")
	.arg(address.toString()).arg(controllingPort).toStdString());

	// Streaming
	_udpSocket = new UdpSocketHandler(this);
	connect(_udpSocket, SIGNAL(tokenAuthorization(const QString&, const QHostAddress&, quint16)), SLOT(onUdpTokenAuthorization(const QString&, const QHostAddress&, quint16)));
	connect(_udpSocket, SIGNAL(done()), _udpSocket, SLOT(deleteLater()));
	if (!_udpSocket->listen(address, streamingPort)) {
		HL_ERROR(HL, QString("Could not open socket for listening (protocol=UDP; address=%1; port=%2)")
		.arg(address.toString()).arg(streamingPort).toStdString());
		return false;
	}
	HL_DEBUG(HL, QString("Bound socket (protocol=UDP; address=%1; port=%2)")
	.arg(address.toString()).arg(streamingPort).toStdString());
	return true;
}

void StreamingServer::quit()
{
	if (QTcpServer::isListening()) {
		QTcpServer::close();
	}

	_modules.clear();
	_userDatabase.clear();

	QHashIterator<Protocol::client_id_t, ClientInfoData> itr(_clients);
	while (itr.hasNext()) {
		itr.next();
		auto socket = itr.value().socketHandler;
		if (socket) {
			socket->disconnect(this);
			socket->close();
		}
	}

	_clients.clear();
	_channels.clear();
	_fingerprint2client.clear();

	if (_udpSocket) {
		_udpSocket->close();
	}
	_udpAuthTokens.clear();
}

QSharedPointer<UserDatabase> StreamingServer::getUserDatabase() const
{
	return _userDatabase;
}

QSharedPointer<MessageDatabase> StreamingServer::getMessageDatabase() const
{
	return _messageDatabase;
}

QSharedPointer<ChannelInfo> StreamingServer::createChannel(const QString &name, const QString &password, bool permanent)
{
	HL_DEBUG(HL, QString("Create channel (name=%1)").arg(name).toStdString());

	// TODO(mfreiholz) Find a Media-Streaming-Node which will host the channel.
	// Save the server information into the ChannelInfo.
//	QString address = "insanefactory.com";
//	quint16 port = Protocol::DefaultStreamingPort;

	// Create new channel.
	QSharedPointer<ChannelInfo> ch(new ChannelInfo());
	ch->id = ChannelInfo::nextUniqueId();
	ch->name = name.toUtf8();
	ch->password = password.toUtf8();
	ch->permanent = permanent;

	ChannelInfoData cdata;
	cdata.info = ch;

	_channels.insert(ch->id, cdata);
	return ch;
}

bool StreamingServer::joinChannel(Protocol::client_id_t clientId, Protocol::channel_id_t channelId)
{
	HL_DEBUG(HL, QString("Client joins channel (client-id=%1; channel-id=%2)").arg(clientId).arg(channelId).toStdString());
	if (!_clients.contains(clientId) || !_channels.contains(channelId)) {
		HL_ERROR(HL, QString("Client can not join channel, invalid IDs.").toStdString());
		return false;
	}

	auto &clientData = _clients[clientId];
	auto &channelData = _channels[channelId];

	if (!clientData.channels.contains(channelData.info)) {
		clientData.channels.append(channelData.info);
	}
	if (!channelData.clients.contains(clientData.info)) {
		channelData.clients.append(clientData.info);
	}
	return true;
}

bool StreamingServer::leaveChannel(Protocol::client_id_t clientId, Protocol::channel_id_t channelId)
{
	HL_DEBUG(HL, QString("Client leaves channel (client-id=%1; channel-id=%2)").arg(clientId).arg(channelId).toStdString());
	if (!_clients.contains(clientId) || !_channels.contains(channelId)) {
		HL_ERROR(HL, QString("Client can not leave channel, invalid IDs.").toStdString());
		return false;
	}

	auto &clientData = _clients[clientId];
	auto &channelData = _channels[channelId];

	clientData.channels.removeAll(channelData.info);
	channelData.clients.removeAll(clientData.info);

	// Delete non-permanent channel, if it has no more clients.
	if (channelData.clients.count() <= 0 && !channelData.info->permanent) {
		_channels.remove(channelId);
	}
	return true;
}

bool StreamingServer::leaveAllChannels(Protocol::client_id_t clientId)
{
	HL_DEBUG(HL, QString("Client leaves all channels (id=%1)").arg(clientId).toStdString());
	if (!_clients.contains(clientId)) {
		return false;
	}

	auto channels = getChannelsOfClient(clientId);
	foreach (auto channel, channels) {
		leaveChannel(clientId, channel->id);
	}
	return true;
}

bool StreamingServer::proveChannelAuthentication(Protocol::channel_id_t channelId, const QByteArray &password)
{
	HL_DEBUG(HL, QString("Prove channel authentication (id=%1)").arg(channelId).toStdString());
	if (!_channels.contains(channelId)) {
		return false;
	}
	auto &channelData = _channels[channelId];
	if (channelData.info->password.isEmpty()) {
		return true;
	} else if (channelData.info->password == password) {
		return true;
	}
	return false;
}

QSharedPointer<ClientInfo> StreamingServer::findClientById(Protocol::client_id_t id) const
{
	QSharedPointer<ClientInfo> foundClient;
	if (_clients.contains(id)) {
		foundClient = _clients[id].info;
	}
	return foundClient;
}

QSharedPointer<ClientInfo> StreamingServer::findClientByFingerprint(const QByteArray &fingerprint) const
{
	return _fingerprint2client.value(fingerprint).toStrongRef();
}

QSharedPointer<ChannelInfo> StreamingServer::findChannelById(Protocol::channel_id_t id) const
{
	QSharedPointer<ChannelInfo> foundChannel;
	if (_channels.contains(id)) {
		foundChannel = _channels[id].info;
	}
	return foundChannel;
}

TcpSocketHandler* StreamingServer::findSocket(Protocol::client_id_t id) const
{
	if (_clients.contains(id)) {
		auto &clientData = _clients[id];
		return clientData.socketHandler;
	}
	return nullptr;
}

TcpSocketHandler* StreamingServer::findSocket(const QByteArray &fingerprint) const
{
	auto clientInfo = _fingerprint2client.value(fingerprint).toStrongRef();
	if (clientInfo) {
		return findSocket(clientInfo->id);
	}
	return nullptr;
}

TcpSocketHandler* StreamingServer::findSocket(QSharedPointer<ClientInfo> clientInfo) const
{
	return findSocket(clientInfo->id);
}

QString StreamingServer::generateUdpAuthToken(QSharedPointer<ClientInfo> ci)
{
	while (true) {
		const QString token = QDateTime::currentDateTime().toString(Qt::ISODate);
		if (_udpAuthTokens.contains(token))
			continue;
		_udpAuthTokens.insert(token, QWeakPointer<ClientInfo>(ci));
		return token;
	}
}

QSharedPointer<ClientInfo> StreamingServer::getClientForUdpAuthToken(const QString &token) const
{
	QSharedPointer<ClientInfo> client_info;
	if (_udpAuthTokens.contains(token)) {
		auto weak = _udpAuthTokens.value(token);
		client_info = weak.toStrongRef();
	}
	return client_info;
}

QList<QSharedPointer<ChannelInfo> > StreamingServer::getChannelsOfClient(Protocol::client_id_t id)
{
	QList<QSharedPointer<ChannelInfo> > channels;
	if (_clients.contains(id)) {
		QMutableListIterator<QWeakPointer<ChannelInfo> > itr(_clients[id].channels);
		while (itr.hasNext()) {
			itr.next();
			auto channel = itr.value().toStrongRef();
			if (channel) {
				channels.append(channel);
			} else {
				itr.remove();
			}
		}
	}
	return channels;
}

QList<QSharedPointer<ClientInfo> > StreamingServer::getClientsOfChannel(Protocol::channel_id_t id)
{
	QList<QSharedPointer<ClientInfo> > clients;
	if (_channels.contains(id)) {
		QMutableListIterator<QWeakPointer<ClientInfo> > itr(_channels[id].clients);
		while (itr.hasNext()) {
			itr.next();
			auto client = itr.value().toStrongRef();
			if (client) {
				clients.append(client);
			} else {
				itr.remove();
			}
		}
	}
	return clients;
}

QList<QSharedPointer<ClientInfo> > StreamingServer::getSiblingClients(Protocol::client_id_t id) const
{
	QList<QSharedPointer<ClientInfo> > siblings;
	if (!_clients.contains(id)) {
		return siblings;
	}
	auto channels = _clients[id].channels;
	foreach (auto weak, channels) {
		auto channel = weak.toStrongRef();
		if (channel) {
			auto clients = _channels[channel->id].clients;
			foreach (auto weak2, clients) {
				auto client = weak2.toStrongRef();
				if (client) {
					siblings.append(client);
				}
			}
		}
	}
	return siblings;
}

void StreamingServer::updateReceiverList()
{
	UdpDataReceiverMap map;

	QHashIterator<Protocol::client_id_t, ClientInfoData> itr(_clients);
	while (itr.hasNext()) {
		itr.next();

		auto ci = itr.value().info;
		if (ci->streamingPort == 0)
			continue;

		const QString ci_identifier = CreateUdpDataReceiverMapKey(QHostAddress(QString(ci->address)), ci->streamingPort);
		map[ci_identifier].client_id = ci->id;

		// iterate through the channels of the current client "ci".
		auto channels = itr.value().channels;
		foreach (auto weakChannel, channels) {
			auto channel = weakChannel.toStrongRef();
			if (!channel || !_channels.contains(channel->id)) {
				continue;
			}

			// now over all clients in the channels, thats or receivers.
			auto channelClients = _channels[channel->id].clients;
			foreach (auto weakChannelClient, channelClients) {
				auto ci_receiver = weakChannelClient.toStrongRef();
				if (!ci_receiver || ci_receiver == ci || ci_receiver->streamingPort == 0)
					continue;
				UdpDataReceiver r;
				r.client_id  = ci_receiver->id;
				r.identifier = CreateUdpDataReceiverMapKey(QHostAddress(QString(ci_receiver->address)), ci_receiver->streamingPort);
				r.address    = QHostAddress(QString(ci_receiver->address));
				r.port       = ci_receiver->streamingPort;
				map[ci_identifier].receivers.append(r);
			}
		}
	}
	_udpSocket->setReceiverMap(map);

	// Log receiver list.
	if (!map.isEmpty()) {
		HL_DEBUG(HL, "Begin of receiver map log");
		const QList<QString> client_ids = map.keys();
		for (int i = 0; i < client_ids.size(); ++i) {
			HL_DEBUG(HL, QString("client(%1)").arg(client_ids[i]).toStdString());
			const UdpDataSender &sender_data = map[client_ids[i]];
			for (int j = 0; j < sender_data.receivers.size(); ++j) {
				HL_DEBUG(HL, QString("=> %1:%2")
				.arg(sender_data.receivers[j].address.toString()).arg(sender_data.receivers[j].port).toStdString());
			}
		}
		HL_DEBUG(HL, "End of receiver map log");
	}
}

// Protected methods //////////////////////////////////////////////////////////

void StreamingServer::incomingConnection(qintptr descriptor)
{
	HL_DEBUG(HL, QString("Incoming connection (descriptor=%1)").arg(descriptor).toStdString());

	ClientInfoData cdata;
	cdata.info = QSharedPointer<ClientInfo>(new ClientInfo());
	cdata.info->id = ClientInfo::nextUniqueId();
	cdata.socketHandler = new TcpSocketHandler(this, cdata.info);
	cdata.socketHandler->open(descriptor);

	_clients.insert(cdata.info->id, cdata);
}

// Private methods ////////////////////////////////////////////////////////////

bool StreamingServer::initUserDatabase()
{
	const QString dbType = _config->value("common/database", "sqlite").toString();
	if (dbType == "sqlite") {
		auto db = new SQLiteUserDatabase();
		if (db->initialize()) {
			_userDatabase = QSharedPointer<UserDatabase>(db);
			return true;
		}
		return false;
	}
	HL_ERROR(HL, QString("Unknown database type '%1'.").arg(dbType).toStdString());
	return false;
}

bool StreamingServer::initMessageDatabase()
{
	const QString dbType = _config->value("common/database", "sqlite").toString();
	if (dbType == "sqlite") {
		auto db = new SQLiteMessageDatabase();
		if (db->initialize()) {
			_messageDatabase = QSharedPointer<MessageDatabase>(db);
			return true;
		}
		return false;
	}
	HL_ERROR(HL, QString("Unknown database type '%1'.").arg(dbType).toStdString());
	return false;
}

// Private Slots //////////////////////////////////////////////////////////////

void StreamingServer::onUdpTokenAuthorization(const QString &token, const QHostAddress &sender, quint16 senderPort)
{
	if (token.isEmpty())
		return;

	auto client = getClientForUdpAuthToken(token);
	if (!client)
		return;

	client->streamingPort = senderPort;
	auto sh = findSocket(client);
	if (!sh)
		return;

	sh->sendUdpAuthenticatedNotification();
	updateReceiverList();
}
