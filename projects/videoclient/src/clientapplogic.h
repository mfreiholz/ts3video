#ifndef CLIENTAPPLOGIC_H
#define CLIENTAPPLOGIC_H

#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QMainWindow>
#include <QAudioInput>

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
		// The camera's device ID to stream video.
		QString cameraDeviceId = QString();
		bool cameraAutoEnable = false;

		// The microphones device ID (audio-in).
		QString audioInputDeviceId = QString();
		bool audioInputAutoEnable = false;

		// The headphones device ID (audio-out).
		QString audioOutputDeviceId = QString();
		bool audioOutputAutoEnable = false;
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
	QSharedPointer<NetworkClient> networkClient();
	QSharedPointer<QAudioInput> audioInput();

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