#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QMainWindow>

#include "videolib/src/ts3video.h"
#include "videolib/src/yuvframe.h"

#include "networkclient/networkclient.h"

class QWidget;
class QProgressDialog;
class ViewBase;
class ClientCameraVideoWidget;
class RemoteClientVideoWidget;


class ClientAppLogicPrivate;
class ClientAppLogic : public QMainWindow
{
	Q_OBJECT
	friend class ClientAppLogicPrivate;
	QScopedPointer<ClientAppLogicPrivate> d;

public:
	class Options
	{
	public:
		// The address and port of the remote server.
		// The address can either be a name like "myhost.com" or and IPv4/IPv6 address.
		QString serverAddress = IFVS_SERVER_ADDRESS;
		quint16 serverPort = IFVS_SERVER_CONNECTION_PORT;
		QString serverPassword;

		// The visible username of the client.
		QString username;

		// Information about the channel to join.
		qint64 channelId = 0;                                              ///< The channel's ID.
		QString channelIdentifier;                                         ///< String based identifier. An ID will be calculated from this string.
		QString channelPassword;                                           ///< The channel's password.

		// The camera's device ID to stream video.
		QString cameraDeviceId;

		// Custom parameters for authentication.
		QHash<QString, QVariant> authParams;
	};

	/*!
	    This is very, very dirty.
	    I'm currently using this approach, because i don't want to pass
	    the object into every small object/dialog which might need it.

	    This method does not actually create the object.
	    As soon as the normal constructor gets called it will return the
	    created instance. As i said... very dirty.

	    \todo As long as we use this way, we can't not have multiple instances.
	*/
	static ClientAppLogic* instance();

public:
	ClientAppLogic(const Options& opts, const QSharedPointer<NetworkClient>& nc, QWidget* parent, Qt::WindowFlags flags);
	virtual ~ClientAppLogic();
	void start();
	QSharedPointer<NetworkClient> networkClient();

private slots:
	void onError(QAbstractSocket::SocketError socketError);
	void onServerError(int code, const QString& message);
	void onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel);
	void onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel);
	void onClientDisconnected(const ClientEntity& client);
	void onNewVideoFrame(YuvFrameRefPtr frame, int senderId);

protected:
	virtual void closeEvent(QCloseEvent* e);
	void showResponseError(int status, const QString& errorMessage, const QString& details = QString());
	void showError(const QString& shortText, const QString& longText = QString());
};


#endif