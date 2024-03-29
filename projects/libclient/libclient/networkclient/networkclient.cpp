#include "networkclient_p.h"

#include <QDataStream>
#include <QHostAddress>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>
#include <QTimerEvent>
#include <QUdpSocket>

#include "humblelogging/api.h"

#include "libqtcorprotocol/qcorconnection.h"
#include "libqtcorprotocol/qcorreply.h"

#include "libapp/timeutil.h"
#include "libapp/ts3video.h"

#include "mediasocket.h"

HUMBLE_LOGGER(HL, "networkclient");

///////////////////////////////////////////////////////////////////////

#define REQUEST_PRECHECK                                                                               \
	if (!d->corSocket->socket() || d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) \
	{                                                                                                  \
		HL_ERROR(HL, QString("Connection is not established.").toStdString());                         \
		return nullptr;                                                                                \
	}

#define REQUEST_PRECHECK_VOID                                                                          \
	if (!d->corSocket->socket() || d->corSocket->socket()->state() != QAbstractSocket::ConnectedState) \
	{                                                                                                  \
		HL_ERROR(HL, QString("Connection is not established.").toStdString());                         \
		return;                                                                                        \
	}

///////////////////////////////////////////////////////////////////////
// Private Implementation
///////////////////////////////////////////////////////////////////////

void NetworkClientPrivate::reset()
{
	heartbeatTimer.stop();
	goodbye = false;
	clientEntity = ClientEntity();
	authToken.clear();
	isAdmin = false;
	serverConfig = VirtualServerConfigEntity();
}

void NetworkClientPrivate::onAuthFinished()
{
	auto d = this;
	auto reply = qobject_cast<QCorReply*>(sender());

	int status = 0;
	QString error;
	QJsonObject params;
	if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, error))
		return;
	else if (status != 0)
		return;

	// Parse self client info and media-authentication-token from response.
	d->clientEntity.fromQJsonObject(params["client"].toObject());
	d->authToken = params["authtoken"].toString();
	d->serverConfig.fromQJsonObject(params["virtualserverconfig"].toObject());

	// Create new media socket.
	d->owner->initMediaSocket();
}

void NetworkClientPrivate::onJoinChannelFinished()
{
	auto d = this;
	auto reply = qobject_cast<QCorReply*>(sender());

	int status = 0;
	QString error;
	QJsonObject params;
	if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, error))
		return;
	else if (status != 0)
		return;

	// Extract channel.
	ChannelEntity channel;
	channel.fromQJsonObject(params["channel"].toObject());
	d->clientModel->setChannel(channel);

	// Extract participants and create widgets.
	foreach(auto v, params["participants"].toArray())
	{
		ClientEntity client;
		client.fromQJsonObject(v.toObject());
		d->clientModel->addClient(client);
	}
}

///////////////////////////////////////////////////////////////////////
// NetworkClient
///////////////////////////////////////////////////////////////////////

NetworkClient::NetworkClient(QObject* parent)
	: QObject(parent)
	, d(new NetworkClientPrivate(this))
{
	qRegisterMetaType<YuvFrameRefPtr>("YuvFrameRefPtr");
	qRegisterMetaType<PcmFrameRefPtr>("PcmFrameRefPtr");
	qRegisterMetaType<NetworkUsageEntity>("NetworkUsageEntity");

	d->corSocket = new QCorConnection(this);
	connect(d->corSocket, &QCorConnection::stateChanged, this, &NetworkClient::onStateChanged);
	connect(d->corSocket, &QCorConnection::error, this, &NetworkClient::onError);
	connect(d->corSocket, &QCorConnection::newIncomingRequest, this, &NetworkClient::onNewIncomingRequest);

	d->heartbeatTimer.setInterval(10000);
	QObject::connect(&d->heartbeatTimer, &QTimer::timeout, this, &NetworkClient::sendHeartbeat);

	d->clientModel.reset(new ClientListModel(this));
	d->clientModel->setNetworkClient(this);

	d->reset();
}

NetworkClient::~NetworkClient()
{
	delete d->corSocket;
	delete d->mediaSocket;
}

const QAbstractSocket* NetworkClient::socket() const
{
	return d->corSocket->socket();
}

const ClientEntity& NetworkClient::clientEntity() const
{
	return d->clientEntity;
}

const VirtualServerConfigEntity& NetworkClient::serverConfig() const
{
	return d->serverConfig;
}

bool NetworkClient::isReadyForStreaming() const
{
	if (!d->mediaSocket || d->mediaSocket->state() != QAbstractSocket::ConnectedState || !d->mediaSocket->isAuthenticated())
		return false;
	return true;
}

bool NetworkClient::isAdmin() const
{
	return d->isAdmin;
}

bool NetworkClient::isSelf(const ClientEntity& ci) const
{
	return d->clientEntity.id == ci.id;
}

ClientListModel* NetworkClient::clientModel() const
{
	return d->clientModel.data();
}

void NetworkClient::connectToHost(const QHostAddress& address, qint16 port)
{
	HL_DEBUG(HL, QString("Connect to server (address=%1; port=%2)").arg(address.toString()).arg(port).toStdString());
	d->corSocket->connectTo(address, port);
}

QCorReply* NetworkClient::auth(const QString& name, const QString& password, const QHash<QString, QVariant>& custom)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Authenticate with server (version=%1; supportedversions=%2; username=%3; password=<HIDDEN>)")
					 .arg(IFVS_SOFTWARE_VERSION)
					 .arg(IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS)
					 .arg(name)
					 .toStdString());

	if (name.isEmpty())
	{
		HL_ERROR(HL, QString("Empty username for authentication not allowed.").toStdString());
		return nullptr;
	}

	// Prepare authentication request.
	QJsonObject params;
	params["version"] = IFVS_SOFTWARE_VERSION;
	params["supportedversions"] = IFVS_CLIENT_SUPPORTED_SERVER_VERSIONS;
	params["username"] = name;
	params["password"] = password;

	// Add custom parameters.
	for (auto i = custom.constBegin(); i != custom.constEnd(); ++i)
	{
		if (!params.contains(i.key()))
			params[i.key()] = QJsonValue::fromVariant(i.value());
	}

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("auth", params));
	auto reply = d->corSocket->sendRequest(req);
	QObject::connect(reply, &QCorReply::finished, d.data(), &NetworkClientPrivate::onAuthFinished);
	return reply;
}

QCorReply* NetworkClient::goodbye()
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Send goodbye").toStdString());

	d->goodbye = true;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("goodbye", QJsonObject()));
	return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::getChannelList(int offset, int limit)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Get channel list (offset=%1; limit=%2)").arg(offset).arg(limit).toStdString());

	QJsonObject params;
	params["offset"] = offset;
	params["limit"] = limit;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("GetChannelList", params));
	auto reply = d->corSocket->sendRequest(req);
	return reply;
}

QCorReply* NetworkClient::joinChannel(ocs::channelid_t id, const QString& password)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Join channel (id=%1; password=<HIDDEN>)").arg(id).toStdString());

	QJsonObject params;
	params["channelid"] = id;
	params["password"] = password;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("joinchannel", params));
	auto reply = d->corSocket->sendRequest(req);
	QObject::connect(reply, &QCorReply::finished, d.data(), &NetworkClientPrivate::onJoinChannelFinished);
	return reply;
}

QCorReply* NetworkClient::joinChannelByIdentifier(const QString& ident, const QString& password)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Join channel (ident=%1; password=<HIDDEN>)").arg(ident).toStdString());

	QJsonObject params;
	params["identifier"] = ident;
	params["password"] = password;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("joinchannelbyidentifier", params));
	auto reply = d->corSocket->sendRequest(req);
	QObject::connect(reply, &QCorReply::finished, d.data(), &NetworkClientPrivate::onJoinChannelFinished);
	return reply;
}

QCorReply* NetworkClient::enableVideoStream(int width, int height, int bitrate)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Enable video stream").toStdString());

	d->clientEntity.videoEnabled = true;
	d->clientEntity.videoWidth = width;
	d->clientEntity.videoHeight = height;
	d->clientEntity.videoBitrate = bitrate;
	d->clientModel->updateClient(d->clientEntity);

	if (d->mediaSocket)
		d->mediaSocket->initVideoEncoder(width, height, bitrate, 15);

	QJsonObject params;
	params["width"] = width;
	params["height"] = height;
	params["bitrate"] = bitrate;
	params["fps"] = 15;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("clientenablevideo", params));
	return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::disableVideoStream()
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Disable video stream").toStdString());

	d->clientEntity.videoEnabled = false;
	d->clientModel->updateClient(d->clientEntity);

	if (d->mediaSocket)
		d->mediaSocket->resetVideoEncoder();

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("clientdisablevideo", QJsonObject()));
	return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::enableRemoteVideoStream(ocs::clientid_t clientId)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Enable remote video stream (client-id=%1)").arg(clientId).toStdString());

	QJsonObject params;
	params["clientid"] = clientId;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("enableremotevideo", params));
	return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::disableRemoteVideoStream(ocs::clientid_t clientId)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Disable remote video stream (client-id=%1)").arg(clientId).toStdString());

	QJsonObject params;
	params["clientid"] = clientId;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("disableremotevideo", params));
	return d->corSocket->sendRequest(req);
}

void NetworkClient::sendVideoFrame(const QImage& image)
{
	if (!isReadyForStreaming())
		return;
	if (!d->clientEntity.videoEnabled)
		return;
	//if (d->clientModel->rowCount() <= 1)
	//	return;
	d->mediaSocket->sendVideoFrame(image, d->clientEntity.id);
}

#if defined(OCS_INCLUDE_AUDIO)
QCorReply* NetworkClient::enableAudioInputStream()
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Enable audio-input stream").toStdString());

	d->clientEntity.audioInputEnabled = true;
	d->clientModel->updateClient(d->clientEntity);

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("clientenableaudioinput", QJsonObject()));
	return d->corSocket->sendRequest(req);
}

QCorReply* NetworkClient::disableAudioInputStream()
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Disable audio-input stream").toStdString());

	d->clientEntity.audioInputEnabled = false;
	d->clientModel->updateClient(d->clientEntity);

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("clientdisableaudioinput", QJsonObject()));
	return d->corSocket->sendRequest(req);
}

void NetworkClient::sendAudioFrame(const PcmFrameRefPtr& f)
{
	if (!d->mediaSocket || !d->clientEntity.audioInputEnabled)
		return;
	d->mediaSocket->sendAudioFrame(f, d->clientEntity.id);
}
#endif

QCorReply* NetworkClient::authAsAdmin(const QString& password)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Authorize as administrator").toStdString());

	QJsonObject params;
	params["password"] = password;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("adminauth", params));
	auto reply = d->corSocket->sendRequest(req);

	connect(reply, &QCorReply::finished, [this, reply]() {
		int status = 0;
		QString error;
		QJsonObject params;
		if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, error))
			return;
		else if (status != 0)
			return;
		d->isAdmin = true;
	});

	return reply;
}

QCorReply* NetworkClient::kickClient(ocs::clientid_t clientId, bool ban)
{
	REQUEST_PRECHECK

	HL_DEBUG(HL, QString("Kick client (client-id=%1; ban=%2)").arg(clientId).arg(ban).toStdString());

	QJsonObject params;
	params["clientid"] = clientId;
	params["ban"] = ban;

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("kickclient", params));
	return d->corSocket->sendRequest(req);
}

// initMediaSocket Creates a new MediaSocket based on "auth-token".
void NetworkClient::initMediaSocket()
{
	if (d->mediaSocket)
	{
		d->mediaSocket->close();
		delete d->mediaSocket;
		d->mediaSocket = nullptr;
	}

	// Connects to server and authenticates with auth-token.
	d->mediaSocket = new MediaSocket(d->authToken, this);
	d->mediaSocket->connectToHost(d->corSocket->socket()->peerAddress(), d->corSocket->socket()->peerPort());

	QObject::connect(d->mediaSocket, &MediaSocket::newVideoFrame, d->owner, &NetworkClient::newVideoFrame);
#if defined(OCS_INCLUDE_AUDIO)
	QObject::connect(d->mediaSocket, &MediaSocket::newAudioFrame, d->owner, &NetworkClient::newAudioFrame);
#endif
	QObject::connect(d->mediaSocket, &MediaSocket::networkUsageUpdated, d->owner, &NetworkClient::networkUsageUpdated);
}

void NetworkClient::sendHeartbeat()
{
	REQUEST_PRECHECK_VOID

	HL_DEBUG(HL, QString("Send heartbeat").toStdString());

	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("heartbeat", QJsonObject()));
	auto reply = d->corSocket->sendRequest(req);
	QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
}

void NetworkClient::onStateChanged(QAbstractSocket::SocketState state)
{
	HL_DEBUG(HL, QString("Socket connection state changed (state=%1)").arg(state).toStdString());
	switch (state)
	{
		case QAbstractSocket::ConnectedState:
			d->heartbeatTimer.start();
			emit connected();
			break;
		case QAbstractSocket::UnconnectedState:
			if (d->mediaSocket)
			{
				d->mediaSocket->close();
				delete d->mediaSocket;
				d->mediaSocket = nullptr;
			}
			d->heartbeatTimer.stop();
			emit disconnected();
			break;
	}
}

void NetworkClient::onError(QAbstractSocket::SocketError err)
{
	HL_DEBUG(HL, QString("Socket connection error (err=%1; goodbye=%2)").arg(err).arg(d->goodbye).toStdString());
	if (d->goodbye)
	{
		d->goodbye = false;
		return;
	}
	emit error(err);
}

void NetworkClient::onNewIncomingRequest(QCorFrameRefPtr frame)
{
	HL_DEBUG(HL, QString("Incoming request (size=%1): %2").arg(frame->data().size()).arg(QString(frame->data())).toStdString());

	QString action;
	QJsonObject parameters;
	if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, parameters))
	{
		// Invalid protocol.
		QCorFrame res;
		res.initResponse(*frame.data());
		res.setData(JsonProtocolHelper::createJsonResponseError(500, "Invalid protocol format."));
		d->corSocket->sendResponse(res);
		return;
	}

	if (action == "notify.mediaauthsuccess")
	{
		d->mediaSocket->setAuthenticated(true);
		emit mediaSocketAuthenticated();
	}
	else if (action == "notify.clientvideoenabled")
	{
		ClientEntity client;
		client.fromQJsonObject(parameters["client"].toObject());

		if (d->mediaSocket)
			d->mediaSocket->resetVideoDecoderOfClient(client.id);

		emit clientEnabledVideo(client);
	}
	else if (action == "notify.clientvideodisabled")
	{
		ClientEntity client;
		client.fromQJsonObject(parameters["client"].toObject());

		if (d->mediaSocket)
			d->mediaSocket->resetVideoDecoderOfClient(client.id);

		emit clientDisabledVideo(client);
	}
	else if (action == "notify.clientjoinedchannel")
	{
		ChannelEntity channelEntity;
		channelEntity.fromQJsonObject(parameters["channel"].toObject());
		ClientEntity clientEntity;
		clientEntity.fromQJsonObject(parameters["client"].toObject());
		emit clientJoinedChannel(clientEntity, channelEntity);
	}
	else if (action == "notify.clientleftchannel")
	{
		ChannelEntity channelEntity;
		channelEntity.fromQJsonObject(parameters["channel"].toObject());
		ClientEntity clientEntity;
		clientEntity.fromQJsonObject(parameters["client"].toObject());
		emit clientLeftChannel(clientEntity, channelEntity);
	}
	else if (action == "notify.clientdisconnected")
	{
		ClientEntity client;
		client.fromQJsonObject(parameters["client"].toObject());

		if (d->mediaSocket)
			d->mediaSocket->resetVideoDecoderOfClient(client.id);

		emit clientDisconnected(client);
	}
	else if (action == "notify.kicked")
	{
		ClientEntity client;
		client.fromQJsonObject(parameters["client"].toObject());
		emit clientKicked(client);
	}
	else if (action == "error")
	{
		auto errorCode = parameters["code"].toInt();
		auto errorMessage = parameters["message"].toString();
		emit serverError(errorCode, errorMessage);
	}
	else
		HL_WARN(HL, QString("Unknown request action. Please check for available software updates. (action=%1)").arg(action).toStdString());

	QCorFrame res;
	res.initResponse(*frame.data());
	res.setData(JsonProtocolHelper::createJsonResponse(QJsonObject()));
	d->corSocket->sendResponse(res);
}