#include "virtualserver.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "humblelogging/api.h"

#include "qcorconnection.h"

#include "servercliententity.h"
#include "serverchannelentity.h"

#include "clientconnectionhandler.h"

HUMBLE_LOGGER(HL, "server");

///////////////////////////////////////////////////////////////////////

VirtualServer::VirtualServer(const VirtualServerOptions& opts, QObject* parent) :
	QObject(parent),
	_opts(opts),
	_corServer(this),
	_connections(),
	_nextClientId(0),
	_clients(),
	_nextChannelId(0),
	_channels(),
	_participants(),
	_mediaSocketHandler(nullptr),
	_tokens(),
	_wsStatusServer(nullptr)
{
	// Basic actions.
	registerAction(ActionPtr(new AuthenticationAction()));

	// Authenticated actions (RequiresAuthentication).
	if (true)
	{
		registerAction(ActionPtr(new GoodbyeAction()));
		registerAction(ActionPtr(new HeartbeatAction()));

		// Channels, Conferences
		registerAction(ActionPtr(new JoinChannelAction()));
		registerAction(ActionPtr(new JoinChannel2Action()));

		// Video
		registerAction(ActionPtr(new EnableVideoAction()));
		registerAction(ActionPtr(new DisableVideoAction()));
		registerAction(ActionPtr(new EnableRemoteVideoAction()));
		registerAction(ActionPtr(new DisableRemoteVideoAction()));

		// Audio
		registerAction(ActionPtr(new EnableAudioInputAction()));
		registerAction(ActionPtr(new DisableAudioInputAction()));
	}

	// Admin actions (RequiresAdminPrivileges)
	if (true)
	{
		registerAction(ActionPtr(new AdminAuthAction()));
		registerAction(ActionPtr(new KickClientAction()));
	}
}

VirtualServer::~VirtualServer()
{
	delete _mediaSocketHandler;
	delete _wsStatusServer;
}

bool VirtualServer::init()
{
	// Init QCorServer listening for new client connections.
	if (!_corServer.listen(_opts.address, _opts.port))
	{
		HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(_opts.port).toStdString());
		return false;
	}
	connect(&_corServer, &QCorServer::newConnection, this, &VirtualServer::onNewConnection);
	HL_INFO(HL, QString("Listening for client connections (protocol=TCP; address=%1; port=%2)").arg(_opts.address.toString()).arg(_opts.port).toStdString());

	// Init media socket.
	_mediaSocketHandler = new MediaSocketHandler(_opts.address, _opts.port, this);
	if (!_mediaSocketHandler->init())
	{
		return false;
	}
	connect(_mediaSocketHandler, &MediaSocketHandler::tokenAuthentication, this, &VirtualServer::onMediaSocketTokenAuthentication);
	connect(_mediaSocketHandler, &MediaSocketHandler::networkUsageUpdated, this, &VirtualServer::onMediaSocketNetworkUsageUpdated);
	HL_INFO(HL, QString("Listening for media data (protocol=UDP; address=%1; port=%2)").arg(_opts.address.toString()).arg(_opts.port).toStdString());

	// Init status web-socket.
	WebSocketStatusServer::Options wsopts;
	wsopts.address = _opts.wsStatusAddress;
	wsopts.port = _opts.wsStatusPort;
	_wsStatusServer = new WebSocketStatusServer(wsopts, this);
	if (!_wsStatusServer->init())
	{
		HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(wsopts.port).toStdString());
		return false;
	}
	HL_INFO(HL, QString("Listening for web-socket status connections (protocol=TCP; address=%1; port=%2)").arg(wsopts.address.toString()).arg(wsopts.port).toStdString());

	return true;
}

const VirtualServerOptions& VirtualServer::options() const
{
	return _opts;
}

void VirtualServer::updateMediaRecipients()
{
	const auto sendBackOwnVideo = false;

	MediaRecipients recips;
	auto clients = _clients.values();
	foreach (auto client, clients)
	{
		// Validate client for streaming
		if (!client)
			continue;
		else if (client->mediaAddress.isNull() || client->mediaPort <= 0)
			continue;
		else if (!client->videoEnabled && !client->audioInputEnabled)
			continue;

		MediaSenderEntity sender;
		sender.clientId = client->id;
		sender.address = QHostAddress(client->mediaAddress);
		sender.port = client->mediaPort;
		sender.id = MediaSenderEntity::createID(sender.address, sender.port);

		MediaReceiverEntity r;
		r.clientId = client->id;
		r.address = client->mediaAddress;
		r.port = client->mediaPort;
		recips.clientid2receiver.insert(r.clientId, r);

		auto siblingClientIds = getSiblingClientIds(client->id);
		foreach (auto siblingClientId, siblingClientIds)
		{
			auto client2 = _clients.value(siblingClientId);
			if (!client2)
				continue;
			else if (client2->mediaAddress.isNull() || client2->mediaPort <= 0)
				continue;
			else if (client2 == client && !sendBackOwnVideo)
				continue;

			MediaReceiverEntity receiver;
			receiver.clientId = client2->id;
			receiver.address = client2->mediaAddress;
			receiver.port = client2->mediaPort;
			sender.receivers.append(receiver);
		}
		recips.id2sender.insert(sender.id, sender);
	}
	_mediaSocketHandler->setRecipients(recips);
}

ServerChannelEntity* VirtualServer::addClientToChannel(int clientId, int channelId)
{
	Q_ASSERT(_channels.value(channelId) != 0);
	auto channelEntity = _channels.value(channelId);
	_participants[channelEntity->id].insert(clientId);
	_client2channels[clientId].insert(channelEntity->id);
	return channelEntity;
}

void VirtualServer::removeClientFromChannel(int clientId, int channelId)
{
	// Remove from channel.
	_participants[channelId].remove(clientId);
	_client2channels[clientId].remove(channelId);
	// Delete channel and free some resources, if there are no more participants.
	if (_participants[channelId].isEmpty())
	{
		_participants.remove(channelId);
		delete _channels.take(channelId);
	}
	if (_client2channels[clientId].isEmpty())
	{
		_client2channels.remove(clientId);
	}
}

void VirtualServer::removeClientFromChannels(int clientId)
{
	// Find all channels of the client.
	QList<int> channelIds;
	if (_client2channels.contains(clientId))
	{
		channelIds = _client2channels[clientId].toList();
	}
	// Remove from all channels.
	foreach (auto channelId, channelIds)
	{
		removeClientFromChannel(clientId, channelId);
	}
}

QList<int> VirtualServer::getSiblingClientIds(int clientId) const
{
	QSet<int> clientIds;
	// From channels (participants).
	foreach (auto channelId, _client2channels.value(clientId).toList())
	{
		foreach (auto participantId, _participants.value(channelId))
		{
			clientIds.insert(participantId);
		}
	}
	return clientIds.toList();
}

void VirtualServer::ban(const QHostAddress& address)
{
	QDir dir(QCoreApplication::applicationDirPath());
	const auto filePath = dir.filePath("bans.ini");
	QSettings ini(filePath, QSettings::IniFormat);

	ini.beginGroup("ips");
	ini.setValue(address.toString(), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
	ini.endGroup();
}

void VirtualServer::unban(const QHostAddress& address)
{
	QDir dir(QCoreApplication::applicationDirPath());
	const auto filePath = dir.filePath("bans.ini");
	QSettings ini(filePath, QSettings::IniFormat);

	ini.beginGroup("ips");
	ini.remove(address.toString());
	ini.endGroup();
}

bool VirtualServer::isBanned(const QHostAddress& address)
{
	QDir dir(QCoreApplication::applicationDirPath());
	const auto filePath = dir.filePath("bans.ini");
	QSettings ini(filePath, QSettings::IniFormat);

	return ini.contains("ips/" + address.toString());
}

// Private Slots

void VirtualServer::onNewConnection(QCorConnection* c)
{
	auto sc = QSharedPointer<QCorConnection>(c);
	new ClientConnectionHandler(this, sc, nullptr);
}

void VirtualServer::onMediaSocketTokenAuthentication(const QString& token, const QHostAddress& address, quint16 port)
{
	if (!_tokens.contains(token))
	{
		HL_WARN(HL, QString("Received invalid media auth token (token=%1; address=%2; port=%3)").arg(token).arg(address.toString()).arg(port).toStdString());
		return;
	}

	// Find matching client.
	auto clientId = _tokens.take(token);
	auto clientEntity = _clients.value(clientId);
	if (!clientEntity)
	{
		HL_WARN(HL, QString("No matching client-entity for auth token (token=%1; client-id=%2)").arg(token).arg(clientId).toStdString());
		return;
	}

	// Update client-info.
	clientEntity->mediaAddress = address;
	clientEntity->mediaPort = port;

	// Notify client about the successful media authentication.
	auto conn = _connections.value(clientId);
	if (conn)
	{
		conn->sendMediaAuthSuccessNotify();
	}
	this->updateMediaRecipients();
}

void VirtualServer::onMediaSocketNetworkUsageUpdated(const NetworkUsageEntity& networkUsage)
{
	_networkUsageMediaSocket = networkUsage;
}

void VirtualServer::registerAction(const ActionPtr& action)
{
	if (_actions.contains(action->name()))
	{
		return;
	}
	_actions.insert(action->name(), action);
}