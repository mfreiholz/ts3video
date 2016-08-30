#ifndef NetworkClient_H
#define NetworkClient_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QScopedPointer>
#include <QAbstractSocket>

#include "qcorlib/qcorframe.h"
#include "qcorlib/qcorreply.h"
#include "qcorlib/qcorconnection.h"

#include "libbase/defines.h"

#include "videolib/yuvframe.h"
#include "videolib/pcmframe.h"

class QHostAddress;
class ClientEntity;
class ChannelEntity;
class VirtualServerConfigEntity;
class NetworkUsageEntity;
class ClientListModel;

class NetworkClientPrivate;
class NetworkClient : public QObject
{
	Q_OBJECT
	friend class NetworkClientPrivate;
	QScopedPointer<NetworkClientPrivate> d;
	NetworkClient(const NetworkClient& other);
	NetworkClient& operator=(const NetworkClient& other);

public:
	NetworkClient(QObject* parent = nullptr);
	~NetworkClient();

	const QAbstractSocket* socket() const;
	const ClientEntity& clientEntity() const;
	const VirtualServerConfigEntity& serverConfig() const;
	bool isReadyForStreaming() const;
	bool isAdmin() const;
	bool isSelf(const ClientEntity& ci) const;

	/*!
	    Gets the model which holds information about the current conference.
	    Ownership stays with NetworkClient.
	    \return ClientListModel*
	*/
	ClientListModel* clientModel() const;

	/*!
	    Connects to the remote VideoServer.
	    Emits the "connected()" signal when the connection is established.
	*/
	void connectToHost(const QHostAddress& address, qint16 port);

	/*!
	    Authenticates with the server.
	    This action has to be performed as first step, as soon as the connection is established.
	    If the client doesn't authenticate within X seconds, the server will drop the connection.
	    \see connected()
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* auth(const QString& name, const QString& password, const QHash<QString, QVariant>& custom = QHash<QString, QVariant>());

	/*!
	    Sends a goodbye to the server, which tells the server to drop this connection.
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* goodbye();

	/*!
		Gets list of channels.
		Requires an authenticated connection.
		\see auth()
		\return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* getChannelList(int offset = 0, int limit = INT_MAX);

	/*!
	    Joins a channel/room/conference.
	    Requires an authenticated connection.
	    \see auth()
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* joinChannel(ocs::channelid_t id, const QString& password);
	QCorReply* joinChannelByIdentifier(const QString& ident, const QString& password);

	/*!
	    Enables/disables sending of video stream to server.
	    Requires an authenticated connection.
	    \see auth()
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* enableVideoStream(int width, int height, int bitrate);
	QCorReply* disableVideoStream();

	/*!
	    Enables/disables receiving the video of a specific participant.
	    Requires an authenticated connection.
	    \see auth()
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* enableRemoteVideoStream(ocs::clientid_t clientId);
	QCorReply* disableRemoteVideoStream(ocs::clientid_t clientId);

	/*!
	    Sends a single frame to the server, which will then broadcast it to other clients.
	    Internally encodes the image with VPX codec.
	    \thread-safe
	    \param image A single frame of the video.
	*/
	void sendVideoFrame(const QImage& image);

#if defined(OCS_INCLUDE_AUDIO)
	/*!
		Enables/disables sending of audio-input data to server (microphone).
		Requires an authenticated connection.
		\see auth()
		\return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* enableAudioInputStream();
	QCorReply* disableAudioInputStream();

	/*!
		Sends a single frame to the server, which will then broadcast it to other clients.
		Internally encodes the frame with Opus codec.
		\thread-safe
		\param f A single raw audio frame.
	*/
	void sendAudioFrame(const PcmFrameRefPtr& f);
#endif

	/*!
	    Tries to authorize the client as administrator.
	    Requires an authenticated connection.
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* authAsAdmin(const QString& password);

	/*!
	    Kicks another client from the server.
	    Requires an authenticated connection with administration privileges.
	    \see authAsAdmin()
	    \param clientId The client to kick.
	    \param ban Set to "true" to ban the client aswell.
	    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
	*/
	QCorReply* kickClient(ocs::clientid_t clientId, bool ban = false);

protected:
	void initMediaSocket();

signals:
	// Connection based signals.
	void connected();
	void disconnected();
	void error(QAbstractSocket::SocketError socketError);
	void mediaSocketAuthenticated();

	// Protocol based signals.
	void serverError(int errorCode, const QString& errorMessage);

	void clientEnabledVideo(const ClientEntity& client);
	void clientDisabledVideo(const ClientEntity& client);
	void clientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel);
	void clientLeftChannel(const ClientEntity& client, const ChannelEntity& channel);
	void clientKicked(const ClientEntity& client);
	void clientDisconnected(const ClientEntity& client);

	void newVideoFrame(YuvFrameRefPtr frame, ocs::clientid_t senderId);
#if defined(OCS_INCLUDE_AUDIO)
	void newAudioFrame(PcmFrameRefPtr frame, ocs::clientid_t senderId);
#endif

	// Periodically updated information to display.
	void networkUsageUpdated(const NetworkUsageEntity& networkUsage);

private slots:
	void sendHeartbeat();
	void onStateChanged(QAbstractSocket::SocketState state);
	void onError(QAbstractSocket::SocketError error);
	void onNewIncomingRequest(QCorFrameRefPtr frame);
};

#endif // NetworkClient_H
