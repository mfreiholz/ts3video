#include "media.h"

#include "humblelogging/api.h"

#include "videolib/virtualserverconfigentity.h"

HUMBLE_LOGGER(HL, "server.clientconnection.action");

void EnableVideoAction::run(const ActionData& req)
{
	// Validate resolution
	const auto width = req.params["width"].toInt();
	const auto height = req.params["height"].toInt();
	const auto bitrate = req.params["bitrate"].toInt();
	const QSize size(width, height);

	//TODO do not create "config".. instead prove _opts directly
	VirtualServerConfigEntity config;
	config.maxVideoResolutionWidth = req.server->_opts.maximumResolution.width();
	config.maxVideoResolutionHeight = req.server->_opts.maximumResolution.height();
	if (!VirtualServerConfigEntity::isResolutionSupported(config, size))
	{
		HL_WARN(HL, QString("Client tried to enable video with unsupported video settings (width=%1; height=%2; bitrate=0)").arg(width).arg(height).arg(bitrate).toStdString());
		sendDefaultErrorResponse(req, IFVS_STATUS_INVALID_PARAMETERS, QString("Unsupported video settings by server (%1x%2 @ %3 bps)").arg(width).arg(height).arg(bitrate));
		return;
	}

	req.session->_clientEntity->videoEnabled = true;
	req.session->_clientEntity->videoWidth = width;
	req.session->_clientEntity->videoHeight = height;
	req.server->updateMediaRecipients();

	sendDefaultOkResponse(req);

	// Broadcast to sibling clients.
	QJsonObject params;
	params["client"] = req.session->_clientEntity->toQJsonObject();
	broadcastNotificationToSiblingClients(req, "notify.clientvideoenabled", params);
}

void DisableVideoAction::run(const ActionData& req)
{
	req.session->_clientEntity->videoEnabled = false;
	req.session->_clientEntity->videoWidth = 0;
	req.session->_clientEntity->videoHeight = 0;
	req.server->updateMediaRecipients();

	sendDefaultOkResponse(req);

	// Broadcast to sibling clients.
	QJsonObject params;
	params["client"] = req.session->_clientEntity->toQJsonObject();
	broadcastNotificationToSiblingClients(req, "notify.clientvideodisabled", params);
}