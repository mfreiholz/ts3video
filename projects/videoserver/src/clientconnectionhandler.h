#ifndef CLIENTCONNECTIONHANDLER_H
#define CLIENTCONNECTIONHANDLER_H

#include <QObject>
#include <QSharedPointer>
#include <QAbstractSocket>
#include <QTime>
#include <QTimer>

#include "libqtcorprotocol/qcorframe.h"

#include "videolib/networkusageentity.h"

class QCorConnection;
class VirtualServer;
class ServerClientEntity;

/*
	JSON Example Request
	--------------------
	{
		"action": "auth",
		"parameters": {
			"key1": "value1",
			"key2": [1, 2, 3]
		}
	}

	JSON Example Response
	---------------------
	{
		"status": 0,  // Required. Status of the response (0 Always means OK!).
		"error": "",  // Optional. Custom error/status message.
		"data": 0     // Optional. Can be everything, defined by action.
	}
*/

/*
	manages the connection and state of a client.
*/
class ClientConnectionHandler : public QObject
{
	Q_OBJECT

public:
	explicit ClientConnectionHandler(VirtualServer* server,
									 QSharedPointer<QCorConnection> connection);
	virtual ~ClientConnectionHandler();
	void sendMediaAuthSuccessNotify();

private slots:
	void onStateChanged(QAbstractSocket::SocketState state);
	void onNewIncomingRequest(QCorFrameRefPtr frame);
	void onAuthenticationTimeout();
	void onConnectionTimeout();

signals:
	void networkUsageUpdated(const NetworkUsageEntity& networkUsage);

public:
	VirtualServer* _server;
	QSharedPointer<QCorConnection> _connection;
	QTimer _connectionTimeoutTimer;

	// information about the client (session data)
	ServerClientEntity* _clientEntity;

	// Network usage.
	NetworkUsageEntity _networkUsage;
	NetworkUsageEntityHelper _networkUsageHelper;
};

#endif