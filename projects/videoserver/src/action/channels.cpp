#include "channels.h"

void GetChannelListAction::run(const ActionData& req)
{
	const auto offset = req.params["offset"].toInt();
	const auto limit = req.params["limit"].toInt();

	if (offset < 0 || offset >= req.server->_id2channel.count() || limit <= 0)
	{
		sendDefaultErrorResponse(req, IFVS_STATUS_INVALID_PARAMETERS, QString("Offset/Limit out of range."));
		return;
	}

	QJsonArray jchannels;
	auto channels = req.server->_id2channel.values();
	qSort(channels.begin(), channels.end(), [](const ServerChannelEntity * a, const ServerChannelEntity * b)
	{
		return a->id < b->id;
	});
	foreach (const auto c, channels)
	{
		jchannels.append(c->toQJsonObject());
	}

	//QJsonArray jchannels;
	//const auto end = offset + limit > req.server->_channels.count() ? req.server->_channels.count() : offset + limit;
	//for (auto i = offset; i < end; ++i)
	//{
	//	const auto& c = req.server->_channels.at(i);
	//	jchannels.append(c->toQJsonObject());
	//}

	QJsonObject params;
	params["channels"] = jchannels;
	sendDefaultOkResponse(req, params);
}

/*
    Joins a channel with different logics.
    1. By it's ID - The channel has to exist.
    2. By it's IDENT-String - The channel will be created, if it doesn't already exists (required by TS3VIDEO).
*/
void JoinChannelAction::run(const ActionData& req)
{
	ocs::channelid_t channelId = req.params["channelid"].toInt();
	const QString channelIdent = req.params["identifier"].toString();
	const QString password = req.params["password"].toString();

	// Find channel ID by IDENT-String
	if (channelId <= 0)
		channelId = req.server->_ident2channel.value(channelIdent);

	// Validate parameters
	if (channelId <= 0 && channelIdent.isEmpty())
	{
		sendDefaultErrorResponse(req, IFVS_STATUS_INVALID_PARAMETERS, QString("Invalid channel identification (channelid=%1; channelident=%2)").arg(channelId).arg(channelIdent));
		return;
	}

	// Retrieve channel
	// Create the channel, if a IDENT-String is given
	auto channelEntity = req.server->_id2channel.value(channelId);
	if (!channelEntity && !channelIdent.isEmpty())
	{
		channelEntity = req.server->createChannel(channelIdent);
		channelEntity->isPasswordProtected = !password.isEmpty();
		channelEntity->password = password;
	}
	if (!channelEntity)
	{
		sendDefaultErrorResponse(req, IFVS_STATUS_INVALID_PARAMETERS, QString("Channel not available (channelid=%1)").arg(channelId));
		return;
	}

	// Verify password.
	if (!req.session->_clientEntity->admin && (!channelEntity->password.isEmpty() && channelEntity->password.compare(password) != 0))
	{
		sendDefaultErrorResponse(req, IFVS_STATUS_UNAUTHORIZED, QString("Wrong channel password (channelid=%1)").arg(channelEntity->id));
		return;
	}

	// Associate the client's membership to the channel.
	req.server->addClientToChannel(req.session->_clientEntity->id, channelEntity->id);
	req.server->updateMediaRecipients();

	// Build response with information about the channel.
	QJsonObject params;
	params["channel"] = channelEntity->toQJsonObject();

	QJsonArray paramsParticipants;
	const auto participantIds = req.server->_participants[channelEntity->id];
	foreach (auto pid, participantIds)
	{
		auto participant = req.server->_clients.value(pid);
		if (!participant)
			continue;
		if (!req.session->_clientEntity->isAllowedToSee(*participant))
			continue;
		paramsParticipants.append(participant->toQJsonObject());
	}
	params["participants"] = paramsParticipants;

	sendDefaultOkResponse(req, params);

	// Notify participants about the new client.
	params = QJsonObject();
	params["channel"] = channelEntity->toQJsonObject();
	params["client"] = req.session->_clientEntity->toQJsonObject();
	broadcastNotificationToSiblingClients(req, "notify.clientjoinedchannel", params);
}

void LeaveChannelAction::run(const ActionData& req)
{
	const ocs::channelid_t channelId = req.params["channelid"].toInt();

	// Find channel.
	auto channelEntity = req.server->_id2channel.value(channelId);
	if (!channelEntity)
	{
		sendDefaultErrorResponse(req, IFVS_STATUS_INVALID_PARAMETERS, QString("Invalid channel id (channelid=%1)").arg(channelId));
		return;
	}

	sendDefaultOkResponse(req);

	// Notify participants.
	QJsonObject params;
	params["channel"] = channelEntity->toQJsonObject();
	params["client"] = req.session->_clientEntity->toQJsonObject();
	broadcastNotificationToSiblingClients(req, "notify.clientleftchannel", params);

	// Leave channel.
	req.server->removeClientFromChannel(req.session->_clientEntity->id, channelId);
	req.server->updateMediaRecipients();
}