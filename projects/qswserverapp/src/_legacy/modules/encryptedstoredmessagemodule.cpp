#include "encryptedstoredmessagemodule.h"

#include "QtCore/QString"
#include "QtCore/QStringList"
#include "QtCore/QVariant"
#include "QtCore/QVariantMap"
#include "QtCore/QDateTime"

#include "humblelogging/api.h"

#include "_legacy/streamingserver.h"
#include "_legacy/tcp_controlling_server_socket_handler.h"

HUMBLE_LOGGER(HL, "network.modules");

/*!
	\class EncryptedStoredMessageModule
	\brief Enables different users to send and receive encrypted messages.

	This module receives and delivers encrypted messages.
	Received messages will be stored on server until the receiver fetches them.
	If the receiver is connected while a message arrives, he will be notified immediately.

	Methods
	=======
	/encryptedstoredmessage/message
		Stores a new encrypted message on server. The server will deliver the message as
		soon as the other client asks for it. If the receiver is connected at the same time
		he will get an notification immediately.

	/encryptedstoredmessage/retrieve
		Retrieves all unread messages from server.

	Notifications
	=============
	/encryptedstoredmessage/message/new
		Server sends this notification, if a new message is available.
		The client should now try to "retrieve" the new message.

	/encryptedstoredmessage/message/delivered
		Server sends this notification, if a messages has been delivered
		to the client. It doesn't mean that the messages has been read.
*/

///////////////////////////////////////////////////////////////////////////////
// Private Data
///////////////////////////////////////////////////////////////////////////////

BEGIN_PRIVATE_IMPL(EncryptedStoredMessageModule)
	void init();
	bool processJsonMessage(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
	bool processJsonCheck(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
END_PRIVATE_IMPL(EncryptedStoredMessageModule)

///////////////////////////////////////////////////////////////////////////////
// EncryptedStoredMessageModule
///////////////////////////////////////////////////////////////////////////////

EncryptedStoredMessageModule::EncryptedStoredMessageModule(StreamingServer *serverBase)
	: BaseModule(serverBase), INIT_PRIVATE(EncryptedStoredMessageModule)
{
}

EncryptedStoredMessageModule::~EncryptedStoredMessageModule()
{
}

QStringList EncryptedStoredMessageModule::getMethodPrefixes() const
{
	return QStringList() << "/encryptedstoredmessage";
}

bool EncryptedStoredMessageModule::processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto json = data.toMap();
	auto method = json.value("method").toString();
	if (method.indexOf("/encryptedstoredmessage/message") == 0) {
		return d->processJsonMessage(socket, header, data);
	} else if (method.indexOf("/encryptedstoredmessage/retrieve") == 0) {
		return d->processJsonCheck(socket, header, data);
	}
	return false;
}

// Private Object Implementation //////////////////////////////////////////////

void EncryptedStoredMessageModule::Private::init()
{
}

bool EncryptedStoredMessageModule::Private::processJsonMessage(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto serverBase = _pOwner->getServerBase();
	auto json = data.toMap();
	auto senderFingerprint = socket._key.fingerprint().toLatin1();
	auto receiverFingerprint = json.value("receiver").toByteArray();
	auto encryptedMessage = json.value("data").toByteArray();

	if (senderFingerprint.isEmpty() || receiverFingerprint.isEmpty() || encryptedMessage.isEmpty()) {
		QVariantMap m;
		m["status"] = 1;
		m["error_message"] = "Invalid fingerprints";
		return socket.sendJsonResponse(header, m);
	}

	// Store message.
	auto messageId = serverBase->getMessageDatabase()->storeMessage(senderFingerprint, receiverFingerprint, encryptedMessage);
	if (messageId <= 0) {
		QVariantMap m;
		m["status"] = 500;
		m["error_message"] = QString("Can not store message");
		return socket.sendJsonResponse(header, m);
	}

	// Send OK response.
	QVariantMap m;
	m["status"] = 0;
	m["message_id"] = messageId;
	auto sent = socket.sendJsonResponse(header, m);

	// Notify the recipient about the new arrived message.
	// Do not include the message content in this notification.
	auto receiverInfo = _pOwner->_serverBase->findClientByFingerprint(receiverFingerprint);
	if (receiverInfo) {
		QVariantMap m;
		m["method"] = "/encryptedstoredmessage/message/new";
		m["sender"] = senderFingerprint;
		auto sock = _pOwner->_serverBase->findSocket(receiverInfo->id);
		if (sock) {
			sock->sendJsonPackage(m);
		}
	}

	return sent;
}

bool EncryptedStoredMessageModule::Private::processJsonCheck(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto serverBase = _pOwner->getServerBase();
	auto receiverFingerprint = socket._key.fingerprint().toLatin1();

	auto messages = serverBase->getMessageDatabase()->getUnreadMessages(receiverFingerprint);
	QVariantList msglist;
	foreach (auto message, messages) {
		QVariantMap msg;
		msg["id"] = message->getId();
		msg["sender_fingerprint"] = message->getSenderFingerprint();
		msg["data"] = message->getMessage();
		msglist.append(msg);
	}

	// Send OK response with messages attached.
	QVariantMap m;
	m["status"] = 0;
	m["messages"] = msglist;
	auto sent = socket.sendJsonResponse(header, m);

	// Mark messages as read (or even delete them?)
	foreach (auto message, messages) {
		serverBase->getMessageDatabase()->markAsRead(message->getId());
	}

	// Notify the "sender" of the message, if he is online.
	// He needs to know that the message has been delivered.
	foreach (auto vt, msglist) {
		auto msg = vt.toMap();
		auto senderFingerprint = msg["sender_fingerprint"].toByteArray();
		auto senderInfo = _pOwner->_serverBase->findClientByFingerprint(senderFingerprint);
		if (senderInfo) {
			auto sock = _pOwner->_serverBase->findSocket(senderInfo->id);
			if (sock) {
				QVariantMap m;
				m["method"] = "/encryptedstoredmessage/message/delivered";
				m["receiver"] = receiverFingerprint;
				m["message_id"] = msg["id"].toUInt();
				sock->sendJsonPackage(m);
			}
		}
	}

	return sent;
}
