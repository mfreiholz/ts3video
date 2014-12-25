#include "_legacy/tcp_controlling_server_socket_handler.h"

#include <functional>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#else
#define NOMINMAX
#include <Winsock2.h>
#endif

#include "QtCore/QStringList"

#include "QtNetwork/QTcpSocket"

#include "QJson/Parser"
#include "QJson/Serializer"

#include "humblelogging/api.h"

#include "shared/network/clientinfo.h"
#include "shared/network/channelinfo.h"
#include "shared/utility/string_utility.h"

#include "_legacy/streamingserver.h"
#include "_legacy/modules/basemodule.h"

HUMBLE_LOGGER(HL, "socket.controlling");

///////////////////////////////////////////////////////////////////////////////
// TcpSocketHandler
///////////////////////////////////////////////////////////////////////////////

TcpSocketHandler::TcpSocketHandler(StreamingServer *serverBase, QSharedPointer<ClientInfo> client, QObject *parent)
	: QObject(parent)
{
	_serverBase = serverBase;
	_state = Idle;
	_socket = nullptr;
	_header = nullptr;
	_buffer.clear();
	_clientInfo = client;
}

TcpSocketHandler::~TcpSocketHandler()
{
	if (_socket)
		_socket->disconnect(this);
}

QSharedPointer<ClientInfo> TcpSocketHandler::getClientInfo() const
{
	return _clientInfo;
}

void TcpSocketHandler::open(qintptr socketDescriptor)
{
	_socket = new QTcpSocket(this);
	connect(_socket, SIGNAL(disconnected()), SLOT(onDisconnected()));
	connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onError(QAbstractSocket::SocketError)));
	connect(_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onStateChanged(QAbstractSocket::SocketState)));
	connect(_socket, SIGNAL(readyRead()), SLOT(onReadyRead()));
	if (!_socket->setSocketDescriptor(socketDescriptor)) {
		HL_ERROR(HL, QString("Could not open socket with descriptor (descriptor=%1)").arg(socketDescriptor).toStdString());
		return;
	}
	const int val = 1;
	::setsockopt(socketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val));

	// Update client information with ip and port.
	_clientInfo->address = _socket->peerAddress().toString().toLatin1();
	_clientInfo->controllingPort = _socket->peerPort();

	HL_DEBUG(HL, QString("Accept client connection (id=%1; address=%2; port=%3)")
	.arg(_clientInfo->id).arg(QString(_clientInfo->address)).arg(_clientInfo->controllingPort).toStdString());
}

void TcpSocketHandler::close()
{
	if (_socket)
		_socket->close();
}

void TcpSocketHandler::onDisconnected()
{
	auto client = _clientInfo;

	// Notify all other related clients.
	auto siblingClients = _serverBase->getSiblingClients(client->id);
	foreach (auto sibClient, siblingClients) {
		auto socket = _serverBase->findSocket(sibClient->id);
		if (socket) {
			socket->sendClientDisconnectedNotification(client);
		}
	}

	_serverBase->leaveAllChannels(client->id);
	_serverBase->_clients.remove(client->id);
	_serverBase->updateReceiverList();

	deleteLater();
}

void TcpSocketHandler::onError(QAbstractSocket::SocketError socketError)
{
	HL_ERROR(HL, QString("Socket error (no=%1; error=%2)")
	.arg(socketError).arg(_socket->errorString()).toStdString());
}

void TcpSocketHandler::onStateChanged(QAbstractSocket::SocketState socketState)
{
	HL_DEBUG(HL, QString("Socket state changed (state=%1")
	.arg(socketState).toStdString());
}

void TcpSocketHandler::onReadyRead()
{
	while (_socket->bytesAvailable() > 0) {
		if (_header == NULL) {
			// wait until enough data for the header is available.
			if (_socket->bytesAvailable() < sizeof(TcpProtocol::request_magic_t) + sizeof(TcpProtocol::RequestHeader)) {
				return;
			}

			QDataStream in(_socket);
			in.setVersion(TcpProtocol::QDS_VERSION);

			// read and check magic number
			TcpProtocol::request_magic_t magic;
			in >> magic;

			if (magic != TcpProtocol::REQUEST_MAGIC_NUMBER) {
				_socket->write("Go home homie, nobody understands you!");
				_socket->disconnectFromHost();
				return;
			}

			// new incoming request
			_header = new TcpProtocol::RequestHeader;
			in >> _header->version;
			in >> _header->correlationId;
			in >> _header->type;
			in >> _header->size;

			if (_header->size > TcpProtocol::REQUEST_MAX_SIZE) {
				_socket->write("Go home homie, nobody wants your crap!");
				_socket->disconnectFromHost();
				return;
			}
			_buffer.reserve(_header->size);
		}

		// read complete request body into buffer.
		if (_buffer.size() < _header->size) {
			_buffer.append( _socket->read(_header->size - _buffer.size()) );
		}

		// does the "_buffer" contains the complete body now?
		if (_buffer.size() != _header->size)
			return;

		processRequest(*_header, _buffer);

		// reset for next request
		delete _header;
		_header = NULL;
		_buffer.clear();
	}
}

bool TcpSocketHandler::processRequest(const TcpProtocol::RequestHeader &header, const QByteArray &body)
{
	// Route the different request types.
	switch (header.type) {
		case TcpProtocol::REQUEST_TYPE_KEEPALIVE: {
			return true;
		}
		case TcpProtocol::REQUEST_TYPE_JSON: {
			return processJsonRequest(header, body);
		}
		default: {
			HL_ERROR(HL, QString("Unknown request package type from client (type=%1)")
			.arg(header.type).toStdString());
			break;
		}
	}
	return false;
}

bool TcpSocketHandler::processJsonRequest(const TcpProtocol::RequestHeader &header, const QByteArray &jsonData)
{
	HL_DEBUG(HL, QString("Incoming JSON request (client=%1; data=%2)")
	.arg(_clientInfo->id).arg(jsonData.data()).toStdString());

	// Parse JSON request.
	bool ok = false;
	QJson::Parser jparser;
	const QVariantMap json = jparser.parse(jsonData, &ok).toMap();
	if (!ok)
		return false;

	// Route JSON "method".
	const QString method = json.value("method").toString();

	if (method == "/goodbye") {
		_socket->disconnectFromHost();
		return true;
	}

	if (method == "/info") {
		QVariantMap m;
		m["status"] = 0;
		m["version"] = "1.0";
		m["supported_versions"] = "1.0";
		m["publickey"] = _serverBase->_key.publicKeyToString();
		m["client_id"] = _clientInfo->id;
		m["streamingtoken"] = _serverBase->generateUdpAuthToken(_clientInfo); // TODO This is only temporary!!!!
		return sendJsonResponse(header, m);
	}

	if (method == "/auth") {
		const QString fingerprint = json.value("fingerprint").toString();

		// Validate paramters.
		if (fingerprint.isEmpty()) {
			QVariantMap m;
			m["status"] = 1;
			return sendJsonResponse(header, m);
		}

		// Check database for fingerprint.
		auto user = _serverBase->getUserDatabase()->findUserByFingerprint(fingerprint.toLatin1());
		if (!user) {
			// Unknown fingerprint, registration required.
			QVariantMap m;
			m["status"] = 2;
			HL_ERROR(HL, QString("Can not find fingerprint").toStdString());
			return sendJsonResponse(header, m);
		}

		// Initialize the local _key object with client's public key.
		if (!_key.importPublicKey(user->getPublicKey())) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Can not import public key into keystore.";
			HL_ERROR(HL, QString("Can not import public key into keystore (publickey-id=%1)").arg(QString(user->getPublicKey())).toStdString());
			return sendJsonResponse(header, m);
		}

		// Validate signature.
		const QByteArray dec = _key.decrypt(QByteArray::fromBase64(json.value("signature").toByteArray()), Key::KT_PUBLIC);
		if (dec.isEmpty() || QLatin1String(dec) != _serverBase->_key.fingerprint()) {
			QVariantMap m;
			m["status"] = 3;
			return sendJsonResponse(header, m);
		}

		// Authentication successfull.
		_state = TcpSocketHandler::Authenticated;
		_serverBase->_fingerprint2client.insert(_key.fingerprint().toLatin1(), _clientInfo);
		_clientInfo->fingerprint = _key.fingerprint().toLatin1();

		QVariantMap m;
		m["status"] = 0;
		m["client_id"] = _clientInfo->id;
		return sendJsonResponse(header, m);
	}
	else if (method == "/auth/register") {
		const QString publicKey = json.value("publickey").toString();

		// Validat parameters.
		if (publicKey.isEmpty()) {
			QVariantMap m;
			m["status"] = 1;
			return sendJsonResponse(header, m);
		}

		// Initialize the local _key object with client's public key.
		if (!_key.importPublicKey(publicKey)) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Can not import public key into keystore.";
			HL_ERROR(HL, QString("Can not import public key into keystore.").toStdString());
			return sendJsonResponse(header, m);
		}

		// Validate signature.
		const QByteArray dec = _key.decrypt(QByteArray::fromBase64(json.value("signature").toByteArray()), Key::KT_PUBLIC);
		if (dec.isEmpty() || QLatin1String(dec) != _serverBase->_key.fingerprint()) {
			QVariantMap m;
			m["status"] = 2;
			return sendJsonResponse(header, m);
		}

		// Registration successful. Insert public-key into database.
		auto user = new UserEntityImpl();
		user->publicKey = _key.publicKeyToString().toLatin1();
		user->fingerprint = _key.fingerprint().toLatin1();
		if (!_serverBase->getUserDatabase()->addUser(UserRef(user))) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Can not insert publickey into database.";
			return sendJsonResponse(header, m);
		}

		// Authentication successfull.
		_state = TcpSocketHandler::Authenticated;
		_serverBase->_fingerprint2client.insert(_key.fingerprint().toLatin1(), _clientInfo);
		_clientInfo->fingerprint = _key.fingerprint().toLatin1();

		QVariantMap m;
		m["status"] = 0;
		m["client_id"] = _clientInfo->id;
		return sendJsonResponse(header, m);
	}

	// At this point, the client must be authorized.
	// Drop the connection, if the client tries something in unauthorized state.
	if (_state != TcpSocketHandler::Authenticated) {
		_socket->disconnectFromHost();
		return false;
	}

	if (method == "/publickey") {
		auto fingerprint = json.value("fingerprint").toByteArray();
		
		// Search public key.
		auto user = _serverBase->getUserDatabase()->findUserByFingerprint(fingerprint);
		if (!user) {
			QVariantMap m;
			m["status"] = 1;
			return sendJsonResponse(header, m);
		}

		// Send OK response.
		QVariantMap m;
		m["status"] = 0;
		m["publickey"] = user->getPublicKey();
		return sendJsonResponse(header, m);
	}

	if (method == "/channel/create") {
		auto name = json.value("name").toString();
		auto password = json.value("password").toString();

		// Create channel.
		auto channelInfo = _serverBase->createChannel(name, password);
		if (!channelInfo) {
			QVariantMap m;
			m["status"] = 1;
			m["error_message"] = "Can not create channel.";
			return sendJsonResponse(header, m);
		}

		// Join the channel.
		if (!_serverBase->joinChannel(_clientInfo->id, channelInfo->id)) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Can not join newly created channel for unknown reason.";
			return sendJsonResponse(header, m);
		}

		// Send OK response.
		QVariantMap m;
		m["status"] = 0;
		m["channel"] = channelInfo->toVariant();
		return sendJsonResponse(header, m);
	}
	else if (method == "/channel/join") {
		const Protocol::channel_id_t channelId = json.value("channel_id").toUInt();
		auto password = json.value("password").toByteArray();

		// Search channel.
		QSharedPointer<ChannelInfo> channelInfo = _serverBase->findChannelById(channelId);
		if (!channelInfo) {
			QVariantMap m;
			m["status"] = 1;
			m["error_message"] = "The channel doesn't exist.";
			return sendJsonResponse(header, m);
		}

		// Check password.
		if (!_serverBase->proveChannelAuthentication(channelId, password)) {
			QVariantMap m;
			m["status"] = 2;
			m["error_message"] = "Wrong password.";
			return sendJsonResponse(header, m);
		}

		// Join channel.
		if (!_serverBase->joinChannel(_clientInfo->id, channelInfo->id)) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Can not join channel for unknown reason.";
			return sendJsonResponse(header, m);
		}

		// Send OK response.
		QVariantMap m;
		m["status"] = 0;
		m["channel"] = channelInfo->toVariant();
		QVariantList l;
		auto participants = _serverBase->getClientsOfChannel(channelInfo->id);
		foreach (auto member, participants) {
			if (member) {
				l.append(member->toVariant());
			}
		}
		m["clients"] = l;
		const bool sent = sendJsonResponse(header, m);

		// Notify channel members about the new client.
		foreach (auto member, participants) {
			if (member && member != _clientInfo) {
				auto socket = _serverBase->findSocket(member->id);
				if (socket) {
					socket->sendClientJoinedChannelNotification(_clientInfo, channelInfo);
				}
			}
		}
		_serverBase->updateReceiverList();
		return sent;
	}
	else if (method == "/channel/leave") {
		const Protocol::channel_id_t channelId = json.value("channel_id").toUInt();

		// Search channel.
		auto channelInfo = _serverBase->findChannelById(channelId);
		if (!channelInfo) {
			QVariantMap m;
			m["status"] = 1;
			m["error_message"] = "Invalid channel ID.";
			return sendJsonResponse(header, m);
		}

		// Leave channel.
		if (!_serverBase->leaveChannel(_clientInfo->id, channelId)) {
			QVariantMap m;
			m["status"] = 500;
			m["error_message"] = "Internal error: Can not leave channel.";
			return sendJsonResponse(header, m);
		}

		// Send OK response.
		QVariantMap m;
		m["status"] = 0;
		auto sent = sendJsonResponse(header, m);

		// Notify all clients in the channel about the leaving client.
		auto participants = _serverBase->getClientsOfChannel(channelInfo->id);
		foreach (auto member, participants) {
			if (member && member != _clientInfo) {
				auto socket = _serverBase->findSocket(member->id);
				if (socket) {
					socket->sendClientLeftChannelNotification(_clientInfo, channelInfo);
				}
			}
		}
		_serverBase->updateReceiverList();
		return sent;
	}
	else if (method == "/channel/list") {
		// Send OK response.
		QVariantList vtChannels;
		QHashIterator<Protocol::channel_id_t, ChannelInfoData> itr(_serverBase->_channels);
		while (itr.hasNext()) {
			itr.next();
			auto &channelData = itr.value();
			channelData.info->memberCount = channelData.clients.count();
			vtChannels.append(itr.value().info->toVariant());
		}
		QVariantMap m;
		m["status"] = 0;
		m["offset"] = 0;
		m["count"] = vtChannels.count();
		m["channels"] = vtChannels;
		return sendJsonResponse(header, m);
	}

	if (method == "core.refreshvideo") {
		const Protocol::client_id_t clientId = json.value("client_id").toUInt();
		auto socket = _serverBase->findSocket(clientId);
		if (socket) {
			socket->sendKeyFrameRequest();
		}
		return true;
	}
	
	if (method == "core.enablestreaming") {
		// Notify sibling clients.
		auto siblingClients = _serverBase->getSiblingClients(_clientInfo->id);
		foreach (auto sib, siblingClients) {
			if (sib && sib != _clientInfo) {
				auto socket = _serverBase->findSocket(sib->id);
				if (socket) {
					socket->sendClientEnabledStreaming(_clientInfo);
				}
			}
		}
		return true;
	}
	
	if (method == "core.disablestreaming") {
		// Notify sibling clients.
		auto siblingClients = _serverBase->getSiblingClients(_clientInfo->id);
		foreach (auto sib, siblingClients) {
			if (sib && sib != _clientInfo) {
				auto socket = _serverBase->findSocket(sib->id);
				if (socket) {
					socket->sendClientDisabledStreaming(_clientInfo);
				}
			}
		}
		return true;
	}
	
	if (method == "/client/list") {
		QVariantList clients;
		QHashIterator<Protocol::client_id_t, ClientInfoData> itr(_serverBase->_clients);
		while (itr.hasNext()) {
			itr.next();
			auto clientInfo = itr.value().info;
			if (clientInfo && clientInfo != _clientInfo) {
				clients.append(clientInfo->toVariant());
			}
		}
		QVariantMap m;
		m["status"] = 0;
		m["clients"] = clients;
		return sendJsonPackage(m);
	}

	// Check modules.
	auto modules = _serverBase->_modules;
	foreach (auto mod, modules) {
		auto prefixes = mod->getMethodPrefixes();
		foreach (auto prefix, prefixes) {
			if (method.startsWith(prefix)) {
				return mod->processJsonRequest(*this, header, json);
			}
		}
	}
	return false;
}

bool TcpSocketHandler::sendRequest(const TcpProtocol::RequestHeader &header, const QByteArray &body)
{
	if (_socket == NULL)
		return false;

	// create request package
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setVersion(TcpProtocol::QDS_VERSION);

	// write magic number
	out << TcpProtocol::REQUEST_MAGIC_NUMBER;

	// write header
	out << header.version;
	out << header.correlationId;
	out << header.type;
	out << header.size;

	// write body
	out.writeRawData(body.constData(), body.size());

	// send via socket
	if (_socket->write(data) == -1) {
		HL_ERROR(HL, QString("Can not send data via socket (error=%1; msg=%2)")
		.arg(_socket->error()).arg(_socket->errorString()).toStdString());
		return false;
	}
	_socket->flush();
	return true;
}

bool TcpSocketHandler::sendJsonPackage(const QByteArray &json)
{
	HL_DEBUG(HL, QString("Outgoing JSON request (client=%1; data=%2)").arg(_clientInfo->id).arg(json.data()).toStdString());
	TcpProtocol::RequestHeader header(TcpProtocol::REQUEST_VERSION, 0, TcpProtocol::REQUEST_TYPE_JSON, json.size());
	return sendRequest(header, json);
}

bool TcpSocketHandler::sendJsonPackage(const QVariant &data)
{
	return sendJsonPackage(QJson::Serializer().serialize(data));
}

bool TcpSocketHandler::sendJsonResponse(const TcpProtocol::RequestHeader &sourceRequest, const QVariant &data)
{
	const QByteArray json = QJson::Serializer().serialize(data);
	TcpProtocol::RequestHeader header(TcpProtocol::REQUEST_VERSION, sourceRequest.correlationId, TcpProtocol::REQUEST_TYPE_JSON, json.size());
	HL_DEBUG(HL, QString("Outgoing JSON response (client=%1; data=%2)").arg(_clientInfo->id).arg(json.data()).toStdString());
	return sendRequest(header, json);
}

bool TcpSocketHandler::sendUdpAuthenticatedNotification()
{
	QVariantMap m;
	m["method"] = "core.authorizedstreaming";
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientInfo(QSharedPointer<ClientInfo> clientInfo)
{
	QVariantMap m;
	m["method"] = "core.updateclient";
	m["success"] = true;
	m["client"] = clientInfo->toVariant();
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientJoinedChannelNotification(
		QSharedPointer<ClientInfo> clientInfo,
		QSharedPointer<ChannelInfo> channelInfo)
{
	QVariantMap m;
	m["method"] = "core.clientjoinedchannel";
	m["channel_id"] = channelInfo->id;
	m["client"] = clientInfo->toVariant();
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientLeftChannelNotification(
		QSharedPointer<ClientInfo> clientInfo,
		QSharedPointer<ChannelInfo> channelInfo)
{
	QVariantMap m;
	m["method"] = "core.clientleftchannel";
	m["channel_id"] = channelInfo->id;
	m["client_id"] = clientInfo->id;
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientConnectedNotification(QSharedPointer<ClientInfo> clientInfo)
{
	QVariantMap m;
	m["method"] = "core.clientconnected";
	m["client"] = clientInfo->toVariant();
	return sendJsonPackage(m);
}

bool TcpSocketHandler::sendClientDisconnectedNotification(
		QSharedPointer<ClientInfo> clientInfo)
{
	QVariantMap m;
	m["method"] = "core.clientdisconnected";
	m["client_id"] = clientInfo->id;
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendKeyFrameRequest()
{
	QVariantMap m;
	m["method"] = "core.refreshvideo";
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientEnabledStreaming(
		QSharedPointer<ClientInfo> clientInfo)
{
	QVariantMap m;
	m["method"] = "core.enablestreaming";
	m["client_id"] = clientInfo->id;
	return sendJsonPackage(QJson::Serializer().serialize(m));
}

bool TcpSocketHandler::sendClientDisabledStreaming(
		QSharedPointer<ClientInfo> clientInfo)
{
	QVariantMap m;
	m["method"] = "core.disablestreaming";
	m["client_id"] = clientInfo->id;
	return sendJsonPackage(QJson::Serializer().serialize(m));
}
