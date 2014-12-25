#include "conferencemodule.h"

#include "QtCore/QString"
#include "QtCore/QStringList"
#include "QtCore/QVariant"
#include "QtCore/QVariantMap"

#include "humblelogging/api.h"

#include "shared/network/protocol.h"

#include "_legacy/streamingserver.h"
#include "_legacy/tcp_controlling_server_socket_handler.h"

HUMBLE_LOGGER(HL, "network.modules");

/*!
	\class ConferenceModule

	Methods
	-------
	/conference/create
		<...>

	/conference/invite
		<...>


	Notifications
	-------------
	/conference/notify/invite
		<...>
*/

///////////////////////////////////////////////////////////////////////////////
// Private Data
///////////////////////////////////////////////////////////////////////////////

BEGIN_PRIVATE_IMPL(ConferenceModule)
  void init();
  bool processJsonConferenceCreate(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
  bool processJsonConferenceInvite(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
END_PRIVATE_IMPL(ConferenceModule)

///////////////////////////////////////////////////////////////////////////////

ConferenceModule::ConferenceModule(StreamingServer *serverBase)
  : BaseModule(serverBase),
    INIT_PRIVATE(ConferenceModule)
{

}

ConferenceModule::~ConferenceModule()
{

}

QStringList ConferenceModule::getMethodPrefixes() const
{
  return QStringList() << "/conference";
}

bool ConferenceModule::processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
  auto json = data.toMap();
  auto method = json.value("method").toString();
  if (method.indexOf("/conference/create") == 0) {
	return d->processJsonConferenceCreate(socket, header, data);
  } else if (method.indexOf("/conference/invite") == 0) {
    return d->processJsonConferenceInvite(socket, header, data);
  }
  return false;
}

// Private Object Implementation //////////////////////////////////////////////

void ConferenceModule::Private::init()
{

}

/*!
  Creates a new private conference and invites additional attendees.
  The conference will get a random password.

  JSON Parameters:
  - participants (Array with fingerprints)
*/
bool ConferenceModule::Private::processJsonConferenceCreate(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
  auto json = data.toMap();
  auto participants = json.value("participants").toList();

  auto server = _pOwner->_serverBase;
  auto selfInfo = socket.getClientInfo();

  // Create invisible channel with random password.
  auto channel = server->createChannel();
  if (!server->joinChannel(selfInfo->id, channel->id)) {
	QVariantMap m;
	m["status"] = 500;
	m["error_message"] = "Can not create channel.";
	return socket.sendJsonResponse(header, m);
  }

  // Invite participants.
  foreach (auto o, participants) {
	auto fingerprint = o.toByteArray();
	auto sock = server->findSocket(fingerprint);
	if (sock) {
		QVariantMap m;
		m["method"] = "/conference/notify/invite";
		m["initiator"] = socket._key.fingerprint();
		m["channel"] = channel->toVariant();
		sock->sendJsonPackage(m);
	}
  }

  // Send OK response.
  QVariantMap m;
  m["status"] = 0;
  m["channel"] = channel->toVariant();
  return socket.sendJsonResponse(header, m);
}

/*!
  Invites additional attendees to an existing conference.
*/
bool ConferenceModule::Private::processJsonConferenceInvite(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
  auto json = data.toMap();
  auto channelId = json.value("channelId").value<Protocol::channel_id_t>();
  auto clientId = json.value("clients").value<Protocol::client_id_t>();

  auto sock = _pOwner->_serverBase->findSocket(clientId);
  if (sock) {
    QVariantMap m;
    m["method"] = "/conference/invitation";
    m["channel_id"] = channelId;
    m["client_id"] = socket.getClientInfo()->id;
    sock->sendJsonPackage(m);
  }

  QVariantMap m;
  m["status"] = 0;
  return socket.sendJsonResponse(header, m);
}
