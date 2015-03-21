#include "profilemodule.h"

#include "QtCore/QString"
#include "QtCore/QStringList"
#include "QtCore/QVariant"
#include "QtCore/QVariantMap"
#include "QtCore/QHash"
#include "QtCore/QDateTime"

#include "humblelogging/api.h"

#include "shared/utility/string_utility.h"

#include "_legacy/streamingserver.h"
#include "_legacy/tcp_controlling_server_socket_handler.h"

HUMBLE_LOGGER(HL, "network.modules");

/*!
	\class ProfileModule
	The ProfileModule provides public information for each registered user.

	Search and find via:
	- Fingerprint
	- E-Mail address
	- Phone number
	- Code (A temporary token)
*/

///////////////////////////////////////////////////////////////////////////////
// Helper objects
//////////////////////////////////////////////////////////////////////////////

struct Token {
	QByteArray _code;
	QByteArray _fingerprint;
	QDateTime  _createdOn;

	Token()
		: _createdOn(QDateTime::currentDateTimeUtc())
	{}

	Token(const QByteArray &code, const QByteArray fingerprint)
		: _code(code),
		  _fingerprint(fingerprint),
		  _createdOn(QDateTime::currentDateTimeUtc())
	{}
};

QVariant toJson(const UserRef &user)
{
	QVariantMap m;
	m["fingerprint"] = user->getFingerprint();
	m["publickey"] = user->getPublicKey();
	m["name"] = user->getName();
	return m;
}

///////////////////////////////////////////////////////////////////////////////
// Private Data
///////////////////////////////////////////////////////////////////////////////

BEGIN_PRIVATE_IMPL(ProfileModule)
	QHash<QByteArray, Token*> _code2token;
	QHash<QByteArray, Token*> _fingerprint2token;

	void init();
	bool processJsonUpdate(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
	bool processJsonGenerateCode(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
	bool processJsonSearchByCode(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
	bool processJsonSearchByFingerprint(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
	bool processJsonSearchByEmail(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
END_PRIVATE_IMPL(ProfileModule)

///////////////////////////////////////////////////////////////////////////////

ProfileModule::ProfileModule(StreamingServer *serverBase)
	: BaseModule(serverBase),
	  INIT_PRIVATE(ProfileModule)
{

}

ProfileModule::~ProfileModule()
{
	d->_code2token.clear();
	qDeleteAll(d->_fingerprint2token);
}

QStringList ProfileModule::getMethodPrefixes() const
{
	return QStringList() << "/profile";
}

bool ProfileModule::processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto json = data.toMap();
	auto method = json.value("method").toString();
	if (method.startsWith("/profile/update")) {
		return d->processJsonUpdate(socket, header, data);
	} else if (method.startsWith("/profile/code")) {
		return d->processJsonGenerateCode(socket, header, data);
	} else if (method.startsWith("/profile/search/bycode")) {
		return d->processJsonSearchByCode(socket, header, data);
	} else if (method.startsWith("/profile/search/byfingerprint")) {
		return d->processJsonSearchByFingerprint(socket, header, data);
	} else if (method.startsWith("/profile/search/byemail")) {
		return d->processJsonSearchByEmail(socket, header, data);
	}
	return false;
}

// Private Object Implementation //////////////////////////////////////////////

void ProfileModule::Private::init()
{

}

/*
 * Updates the user's profile.
 */
bool ProfileModule::Private::processJsonUpdate(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto json = data.toMap();
	auto juser = json.value("user").toMap();

	auto user = new UserEntityImpl();
	user->fingerprint = socket.getClientInfo()->fingerprint;
	user->name = juser.value("name").toString();

	auto serverBase = _pOwner->getServerBase();
	if (!serverBase->getUserDatabase()->updateUser(UserRef(user))) {
		QVariantMap m;
		m["status"] = 500;
		return socket.sendJsonResponse(header, m);
	}
	
	QVariantMap m;
	m["status"] = 0;
	return socket.sendJsonResponse(header, m);
}


/*
 * Generates a temporary code, which can be used by another user to find the user's profile.
 */
bool ProfileModule::Private::processJsonGenerateCode(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto fingerprint = socket.getClientInfo()->fingerprint;
	auto token = _fingerprint2token.value(fingerprint);

	if (!token) {
		QByteArray code;
		do {
			code = generateRandomString(4, true, false, false).toUtf8();
		}
		while (_code2token.contains(code));

		token = new Token(code, fingerprint);
		_code2token.insert(code, token);
		_fingerprint2token.insert(fingerprint, token);
	}

	QVariantMap m;
	m["status"] = 0;
	m["code"] = token->_code;
	return socket.sendJsonResponse(header, m);
}

/*
 * Searches a user's profile by a temporary code.
 */
bool ProfileModule::Private::processJsonSearchByCode(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto serverBase = _pOwner->getServerBase();
	auto json = data.toMap();
	auto code = json.value("code").toByteArray();

	if (code.isEmpty() || !_code2token.contains(code)) {
		QVariantMap m;
		m["status"] = 1;
		return socket.sendJsonResponse(header, m);
	}

	auto token = _code2token.value(code);
	auto user = serverBase->getUserDatabase()->findUserByFingerprint(token->_fingerprint);

	if (!user) {
		QVariantMap m;
		m["status"] = 500;
		return socket.sendJsonResponse(header, m);
	}

	QVariantMap m;
	m["status"] = 0;
	m["code"] = code;
	
	QVariantList results;
	results.append(toJson(user));
	m["results"] = results;
	
	return socket.sendJsonResponse(header, m);
}

/*
 * Searches a user's profile by his fingerprint.
 */
bool ProfileModule::Private::processJsonSearchByFingerprint(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	auto serverBase = _pOwner->getServerBase();
	auto json = data.toMap();
	auto fingerprint = json.value("fingerprint").toByteArray();

	// Validate paramters.
	if (fingerprint.isEmpty()) {
		QVariantMap m;
		m["status"] = 1;
		return socket.sendJsonResponse(header, m);
	}

	// Execute search.
	auto user = serverBase->getUserDatabase()->findUserByFingerprint(fingerprint);
	if (!user) {
		QVariantMap m;
		m["status"] = 2;
		return socket.sendJsonResponse(header, m);
	}

	// Send OK response.
	QVariantMap m;
	m["status"] = 0;

	QVariantList results;
	results.append(toJson(user));
	m["results"] = results;

	return socket.sendJsonResponse(header, m);
}

/*
 * Searches a user's profile by it's email address.
 */
bool ProfileModule::Private::processJsonSearchByEmail(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	return true;
}
