#include "clientconnectionhandler.h"

#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTcpSocket>

#include "humblelogging/api.h"

#include "qcorlib/qcorconnection.h"
#include "qcorlib/qcorreply.h"

#include "ts3video.h"
#include "elws.h"
#include "jsonprotocolhelper.h"

#include "virtualserver.h"
#include "servercliententity.h"
#include "serverchannelentity.h"

HUMBLE_LOGGER(HL, "server.clientconnection");

///////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(VirtualServer* server, QSharedPointer<QCorConnection> connection, QObject* parent) :
	QObject(parent),
	_server(server),
	_connection(connection),
	_clientEntity(nullptr),
	_networkUsage(),
	_networkUsageHelper(_networkUsage)
{
	_clientEntity = new ServerClientEntity();
	_clientEntity->id = ++_server->_nextClientId;
	_server->_clients.insert(_clientEntity->id, _clientEntity);

	_server->_connections.insert(_clientEntity->id, this);
	connect(_connection.data(), &QCorConnection::stateChanged, this, &ClientConnectionHandler::onStateChanged);
	connect(_connection.data(), &QCorConnection::newIncomingRequest, this, &ClientConnectionHandler::onNewIncomingRequest);
	onStateChanged(QAbstractSocket::ConnectedState);

	// Authentication timer: Close connection, if it doesn't authentication within X seconds.
	QTimer::singleShot(60000, this, &ClientConnectionHandler::onAuthenticationTimeout);

	// Connection timer: Close connection, if there wasn't a keep-alive or other package for X seconds.
	_connectionTimeoutTimer.start(20000);
	connect(&_connectionTimeoutTimer, &QTimer::timeout, this, &ClientConnectionHandler::onConnectionTimeout);

	// Statistics timer: Update network statistics every X seconds.
	auto statisticTimer = new QTimer(this);
	statisticTimer->setInterval(2000);
	statisticTimer->start();
	connect(statisticTimer, &QTimer::timeout, [this]()
	{
		_networkUsageHelper.recalculate();
		emit networkUsageUpdated(_networkUsage);
	});
}


ClientConnectionHandler::~ClientConnectionHandler()
{
	_server->removeClientFromChannels(_clientEntity->id);
	_server->_clients.remove(_clientEntity->id);
	_server->_connections.remove(_clientEntity->id);
	delete _clientEntity;
	_connection.clear();
	_clientEntity = nullptr;

	_server->updateMediaRecipients();
	_server = nullptr;
}


void ClientConnectionHandler::sendMediaAuthSuccessNotify()
{
	QCorFrame req;
	req.setData(JsonProtocolHelper::createJsonRequest("notify.mediaauthsuccess", QJsonObject()));
	auto reply = _connection->sendRequest(req);
	QCORREPLY_AUTODELETE(reply);
}


void ClientConnectionHandler::onStateChanged(QAbstractSocket::SocketState state)
{
	switch (state)
	{
	case QAbstractSocket::ConnectedState:
	{
		HL_INFO(HL, QString("New client connection (addr=%1; port=%2; clientid=%3)").arg(_connection->socket()->peerAddress().toString()).arg(_connection->socket()->peerPort()).arg(_clientEntity->id).toStdString());
		break;
	}
	case QAbstractSocket::UnconnectedState:
	{
		HL_INFO(HL, QString("Client disconnected (addr=%1; port=%2; clientid=%3)").arg(_connection->socket()->peerAddress().toString()).arg(_connection->socket()->peerPort()).arg(_clientEntity->id).toStdString());
		// Notify sibling clients about the disconnect.
		QJsonObject params;
		params["client"] = _clientEntity->toQJsonObject();
		QCorFrame req;
		req.setData(JsonProtocolHelper::createJsonRequest("notify.clientdisconnected", params));
		auto clientConns = _server->_connections.values();
		foreach (auto clientConn, clientConns)
		{
			if (clientConn && clientConn != this)
			{
				auto reply = clientConn->_connection->sendRequest(req);
				QCORREPLY_AUTODELETE(reply);
			}
		}
		// Delete itself.
		deleteLater();
		break;
	}
	}
}


void ClientConnectionHandler::onNewIncomingRequest(QCorFrameRefPtr frame)
{
	HL_TRACE(HL, QString("New incoming request (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data())).toStdString());
	_networkUsage.bytesRead += frame->data().size(); // TODO Not correct, we need to get values from QCORLIB to include bytes of cor_frame (same for write).

	// Parse incoming request.
	QString action;
	QJsonObject params;
	if (!JsonProtocolHelper::fromJsonRequest(frame->data(), action, params))
	{
		HL_WARN(HL, QString("Retrieved request with invalid JSON protocol format (data=%1)").arg(QString(frame->data())).toStdString());
		ActionBase::sendErrorResponse(*_connection.data(), *frame.data(), IFVS_STATUS_INTERNAL_ERROR, "Invalid JSON protocol format");
		return;
	}

	// Find matching action handler.
	auto actionHandler = _server->_actions.value(action);
	if (!actionHandler)
	{
		HL_WARN(HL, QString("Retrieved request with unknown action (action=%1)").arg(action).toStdString());
		ActionBase::sendErrorResponse(*_connection.data(), *frame.data(), IFVS_STATUS_NOT_IMPLEMENTED, QString("Action not implemented (action=%1)").arg(action));
		return;
	}

	// The client needs to be authenticated before he can request any action with RequiresAuthentication flag.
	if (actionHandler->flags().testFlag(ActionBase::RequiresAuthentication) && !_clientEntity->authenticated)
	{
		ActionBase::sendErrorResponse(*_connection.data(), *frame.data(), IFVS_STATUS_FORBIDDEN, QString("You are not allowed to perform this action (action=%1)").arg(action));
		return;
	}

	// The client may require admin-privileges for the found action.
	if (actionHandler->flags().testFlag(ActionBase::RequiresAdminPrivileges) && !_clientEntity->admin)
	{
		ActionBase::sendErrorResponse(*_connection.data(), *frame.data(), IFVS_STATUS_FORBIDDEN, QString("You are not allowed to perform this action (action=%1)").arg(action));
		return;
	}

	// The request and its prerequisites seems to be legit
	//   -> process it.
	ActionData req;
	req.server = _server;
	req.session = this;
	req.connection = _connection;
	req.frame = frame;
	req.action = action;
	req.params = params;
	actionHandler->run(req);
}


void ClientConnectionHandler::onAuthenticationTimeout()
{
	if (_clientEntity->authenticated)
		return;
	HL_WARN(HL, QString("Client did not authenticate within 60 seconds.").toStdString());
	ActionBase::sendErrorRequest(*_connection.data(), IFVS_STATUS_AUTHENTICATION_TIMEOUT, "Authentication process took too long. Check your connection and try it again.");
	QMetaObject::invokeMethod(_connection.data(), "disconnectFromHost", Qt::QueuedConnection);
}


void ClientConnectionHandler::onConnectionTimeout()
{
	HL_WARN(HL, QString("Client connection timed out. No heartbeat since 20 seconds.").toStdString());
	ActionBase::sendErrorRequest(*_connection.data(), IFVS_STATUS_CONNECTION_TIMEOUT, "Connection timeout. No heartbeat since 20 seconds.");
	QMetaObject::invokeMethod(_connection.data(), "disconnectFromHost", Qt::QueuedConnection);
}