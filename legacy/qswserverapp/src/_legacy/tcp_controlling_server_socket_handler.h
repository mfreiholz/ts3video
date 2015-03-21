#ifndef _TCPCONTROLLINGSERVERSOCKETHANDLER_HEADER_
#define _TCPCONTROLLINGSERVERSOCKETHANDLER_HEADER_

#include "QtCore/QObject"
#include "QtCore/QSharedPointer"
#include "QtCore/QByteArray"

#include "QtNetwork/QAbstractSocket"

#include "shared/crypto/key.h"
#include "shared/network/protocol.h"
#include "shared/network/tcp_controlling_protocol.h"

class QTcpSocket;
class QVariant;
class StreamingServer;
class ClientInfo;
class ChannelInfo;


/*!
	Handles a single client connection.
*/
class TcpSocketHandler : public QObject
{
	Q_OBJECT
	friend class StreamingServer;
public:
	enum State { Idle, Authenticated };

	TcpSocketHandler(StreamingServer *server, QSharedPointer<ClientInfo> clientInfo, QObject *parent = nullptr);
	~TcpSocketHandler();

	QSharedPointer<ClientInfo> getClientInfo() const;
	void open(qintptr socketDescriptor);
	void close();

private slots:
	// If the state changes into <em>UnconnectedState</em>, it will emit the
	// <em>done()</em> signal.
	void onDisconnected();
	// Gets invoked whenever the internal QTcpSocket has an error.
	// Invoked from internal QTcpSocket
	void onError(QAbstractSocket::SocketError socketError);
	// Gets invoked whenever the internal QTcpSocket changes his state.
	// Invoked from internal QTcpSocket
	void onStateChanged(QAbstractSocket::SocketState socketState);
	// Gets invoked whenever new data is available.
	// Invoked from internal QTcpSocket
	void onReadyRead();

protected:
	/*!
		Every complete incoming packet will proceed this function.
		The function decides by the type of the package what should be done with it.

		\param[in] header
			The incoming packet header.
		\param[in] body
			The complete incoming packet data.
		\return true/false
	*/
	bool processRequest(const TcpProtocol::RequestHeader &header, const QByteArray &body);

	/*!
		All JSON packets will pass this function (TcpProtocol::REQUEST_TYPE_JSON).

		\param[in] header
			The incoming packet header.
		\param[in] body
			The complete JSON data
		\return true/false
	*/
	bool processJsonRequest(const TcpProtocol::RequestHeader &header, const QByteArray &json);

public slots:
	/*!
		Sends the given data (typed defined by the header) to the remote client.

		\param[in] header
		\param[in] body
		\return true/false
	*/
	bool sendRequest(const TcpProtocol::RequestHeader &header, const QByteArray &body);

	/*!
		Sends the given JSON data to remote client. The function automatically
		creates and initializes the required "TcpProtocol::RequestHeader" object
		and calls the "sendRequest()" function.

		\param[in] json
		\return true/false
	*/
	bool sendJsonPackage(const QByteArray &json);
	bool sendJsonPackage(const QVariant &data);

	bool sendJsonResponse(const TcpProtocol::RequestHeader &sourceRequest, const QVariant &data);

protected slots:
	bool sendUdpAuthenticatedNotification();
	bool sendClientInfo(QSharedPointer<ClientInfo> clientInfo);
	bool sendClientJoinedChannelNotification(QSharedPointer<ClientInfo> clientInfo, QSharedPointer<ChannelInfo> channelInfo);
	bool sendClientLeftChannelNotification(QSharedPointer<ClientInfo> clientInfo, QSharedPointer<ChannelInfo> channelInfo);
	bool sendClientConnectedNotification(QSharedPointer<ClientInfo> clientInfo);
	bool sendClientDisconnectedNotification(QSharedPointer<ClientInfo> clientInfo);
	bool sendKeyFrameRequest();
	bool sendClientEnabledStreaming(QSharedPointer<ClientInfo> client_info);
	bool sendClientDisabledStreaming(QSharedPointer<ClientInfo> client_info);

signals:
	void error(QAbstractSocket::SocketError socketError, const QString &errorMessage);

public:
	StreamingServer *_serverBase;
	TcpSocketHandler::State _state;
	
	QTcpSocket *_socket;
	TcpProtocol::RequestHeader *_header;
	QByteArray _buffer;

	QSharedPointer<ClientInfo> _clientInfo;
	Key _key;
};

#endif
