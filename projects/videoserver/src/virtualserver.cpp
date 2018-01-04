#include "virtualserver.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "humblelogging/api.h"

#include "libqtcorprotocol/qcorconnection.h"

#include "libapp/virtualserverconfigentity.h"

#include "action/channels.h"
#include "action/media.h"
#include "mediasockethandler.h"
#include "websocketstatusserver.h"
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
	_participants(),
	_mediaSocketHandler(nullptr),
	_tokens(),
	_wsStatusServer(nullptr)
{
	// Basic actions.
	registerAction(std::make_shared<HeartbeatAction>());
	registerAction(std::make_shared<GoodbyeAction>());
	registerAction(std::make_shared<AuthenticationAction>());

	// Authenticated actions (RequiresAuthentication).
	if (true)
	{
		// Channels, Conferences
		registerAction(std::make_shared<GetChannelListAction>());
		registerAction(std::make_shared<CreateChannelAction>());
		registerAction(std::make_shared<JoinChannelAction>());
		registerAction(std::make_shared<JoinChannel2Action>());

		// Video
		registerAction(std::make_shared<EnableVideoAction>());
		registerAction(std::make_shared<DisableVideoAction>());

		// Audio
		registerAction(std::make_shared<EnableAudioInputAction>());
		registerAction(std::make_shared<DisableAudioInputAction>());
	}

	// Admin actions (RequiresAdminPrivileges)
	if (true)
	{
		registerAction(std::make_shared<AdminAuthAction>());
		registerAction(std::make_shared<KickClientAction>());
		registerAction(std::make_shared<UpdateVisibilityLevelAction>());
		registerAction(std::make_shared<AddDirectStreamingRelationAction>());
		registerAction(std::make_shared<RemoveDirectStreamingRelationAction>());
	}
}

VirtualServer::~VirtualServer()
{
	stop();
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
	//_mediaSocketHandler = std::make_unique<MediaSocketHandler>(_opts.address, _opts.port, this);
	_mediaSocketHandler.reset(new MediaSocketHandler(_opts.address, _opts.port, this));
	if (!_mediaSocketHandler->init())
	{
		return false;
	}
	connect(_mediaSocketHandler.get(), &MediaSocketHandler::tokenAuthentication, this, &VirtualServer::onMediaSocketTokenAuthentication);
	connect(_mediaSocketHandler.get(), &MediaSocketHandler::networkUsageUpdated, this, &VirtualServer::onMediaSocketNetworkUsageUpdated);
	HL_INFO(HL, QString("Listening for media data (protocol=UDP; address=%1; port=%2)").arg(_opts.address.toString()).arg(_opts.port).toStdString());

	// Init status web-socket.
	WebSocketStatusServer::Options wsopts;
	wsopts.address = _opts.wsStatusAddress;
	wsopts.port = _opts.wsStatusPort;
	//_wsStatusServer = std::make_unique<WebSocketStatusServer>(wsopts, this);
	_wsStatusServer.reset(new WebSocketStatusServer(wsopts, this));
	if (!_wsStatusServer->init())
	{
		HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(wsopts.port).toStdString());
		return false;
	}
	HL_INFO(HL, QString("Listening for web-socket status connections (protocol=TCP; address=%1; port=%2)").arg(wsopts.address.toString()).arg(wsopts.port).toStdString());

	return true;
}

void VirtualServer::stop()
{
	// Main
	_corServer.close();
	_corServer.disconnect(this);

	// Media
	_mediaSocketHandler->disconnect(this);
	_mediaSocketHandler.reset();

	// Web socket
	_wsStatusServer->disconnect(this);
	_wsStatusServer.reset();
}

const VirtualServerOptions& VirtualServer::options() const
{
	return _opts;
}

void VirtualServer::updateMediaRecipients()
{
	auto sendBackOwnVideo = false;

	MediaRecipients recips;
	auto clients = _clients.values();
	for (auto client : clients)
	{
		// Create SENDER entity for "client"
		// Validate client for streaming
		if (!client)
			continue;
		else if (client->mediaAddress.isNull() || client->mediaPort <= 0)
			continue;
		else if (!client->videoEnabled && !client->audioInputEnabled)
			continue;

		MediaSenderEntity sender;
		sender.clientId = client->id;
		sender.address = client->mediaAddress;
		sender.port = client->mediaPort;

		// Fill SENDER receiver list - by conference members.
		auto siblingClientIds = getSiblingClientIds(sender.clientId, true);
		for (const auto clientId : siblingClientIds)
		{
			auto c = _clients.value(clientId);
			if (!c)
				continue;
			else if (c->mediaAddress.isNull() || c->mediaPort <= 0)
				continue;
			else if (c == client && !sendBackOwnVideo)
				continue;

			MediaReceiverEntity r;
			r.clientId = c->id;
			r.address = c->mediaAddress;
			r.port = c->mediaPort;
			sender.receivers.append(std::move(r));
		}

		// Fill SENDER receiver list - by direct mappings.
		auto directReceivers = _sender2receiver.value(sender.clientId);
		for (const auto clientId : directReceivers)
		{
			auto c = _clients.value(clientId);
			if (!c)
				continue;
			else if (c->mediaAddress.isNull() || c->mediaPort <= 0)
				continue;
			else if (c == client)
				continue;

			MediaReceiverEntity r;
			r.clientId = c->id;
			r.address = c->mediaAddress;
			r.port = c->mediaPort;
			sender.receivers.append(std::move(r));
		}

		// Create RECEIVER entity for "client".
		MediaReceiverEntity receiver;
		receiver.clientId = client->id;
		receiver.address = client->mediaAddress;
		receiver.port = client->mediaPort;

		// Fill "recips" with created information.
		recips.addr2sender[sender.address][sender.port] = sender;
		recips.clientid2receiver.insert(receiver.clientId, receiver);
	}
	_mediaSocketHandler->setRecipients(std::move(recips));
}

std::shared_ptr<ActionBase> VirtualServer::findHandlerByName(const QString& name) const
{
	return _actions.value(name);
}

ServerChannelEntity* VirtualServer::createChannel(const QString& ident)
{
	auto c = new ServerChannelEntity();
	c->id = ++_nextChannelId;
	c->ident = ident;

	_id2channel.insert(c->id, c);
	if (!c->ident.isEmpty())
		_ident2channel.insert(c->ident, c->id);

	return c;
}

/*  void VirtualServer::deleteChannel(ocs::channelid_t channelId)
    {
	auto channel = _id2channel.take(channelId);
	if (!channel)
	{
		HL_ERROR(HL, QString("invalid channel (id=%1)").arg(channelId).toStdString());
		return;
	}
	delete channel;
	channel = nullptr;

	// notify users if there are still some in channel
	// -> ???


	// clean up mappings
	const auto clientIds = _participants.take(channelId);
	foreach (auto clientId, clientIds)
	{

	}



	if (_participants[channelId].isEmpty())
	{
		_participants.remove(channelId);
		auto c = _id2channel.take(channelId);
		_ident2channel.remove(c->ident);
		delete c;
	}

    }*/

ServerChannelEntity* VirtualServer::addClientToChannel(ocs::clientid_t clientId, ocs::channelid_t channelId)
{
	auto channelEntity = _id2channel.value(channelId);
	if (!channelEntity)
	{
		HL_ERROR(HL, QString("Channel does not exist (channelId=%1)").arg(channelId).toStdString());
		return nullptr;
	}
	_participants[channelEntity->id].insert(clientId);
	_client2channels[clientId].insert(channelEntity->id);
	return channelEntity;
}

void VirtualServer::removeClientFromChannel(ocs::clientid_t clientId, ocs::channelid_t channelId)
{
	// Remove from channel.
	_participants[channelId].remove(clientId);
	_client2channels[clientId].remove(channelId);

	// Delete channel and free some resources, if there are no more participants.
	if (_participants[channelId].isEmpty())
	{
		_participants.remove(channelId);
		auto c = _id2channel.take(channelId);
		_ident2channel.remove(c->ident);
		delete c;
	}
	if (_client2channels[clientId].isEmpty())
	{
		_client2channels.remove(clientId);
	}
}

void VirtualServer::removeClientFromChannels(ocs::clientid_t clientId)
{
	// Find all channels of the client.
	QList<ocs::channelid_t> channelIds;
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

QList<ocs::clientid_t> VirtualServer::getSiblingClientIds(ocs::clientid_t clientId, bool filterByVisibilityLevel) const
{
	QSet<ocs::clientid_t> clientIds;
	foreach (auto channelId, _client2channels.value(clientId).toList())
	{
		foreach (auto participantId, _participants.value(channelId))
		{
			if (filterByVisibilityLevel)
			{
				auto c1 = _clients.value(clientId);
				auto c2 = _clients.value(participantId);
				if (!c1 || !c2 || !c1->isAllowedToSee(*c2))
					continue;
			}
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
	new ClientConnectionHandler(this, sc);
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

void VirtualServer::registerAction(std::shared_ptr<ActionBase> action)
{
	if (_actions.contains(action->name()))
	{
		return;
	}
	_actions.insert(action->name(), action);
}