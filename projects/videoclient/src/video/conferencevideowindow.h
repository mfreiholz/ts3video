#ifndef CONFERENCEVIDEOWINDOW_H
#define CONFERENCEVIDEOWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QHash>
#include <QVariant>

#include "videolib/src/ts3video.h"
#include "videolib/src/yuvframe.h"

#include "networkclient/networkclient.h"

#if defined(OCS_INCLUDE_AUDIO)
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#endif

class QWidget;
class QProgressDialog;
class QCamera;
class ViewBase;
class RemoteClientVideoWidget;

class ConferenceVideoWindow : public QMainWindow
{
	Q_OBJECT

public:
	class Options
	{
	public:
		// The camera's device ID to stream video.
		QString cameraDeviceId = QString();
		bool cameraAutoEnable = false;

#if defined(OCS_INCLUDE_AUDIO)
		// The microphones device ID (audio-in).
		QString audioInputDeviceId = QString();
		bool audioInputAutoEnable = false;

		// The headphones device ID (audio-out).
		QString audioOutputDeviceId = QString();
		bool audioOutputAutoEnable = false;
#endif
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
	static ConferenceVideoWindow* instance();

public:
	ConferenceVideoWindow(const Options& opts, const QSharedPointer<NetworkClient>& nc, QWidget* parent, Qt::WindowFlags flags);
	virtual ~ConferenceVideoWindow();
	QSharedPointer<NetworkClient> networkClient();

#if defined(OCS_INCLUDE_AUDIO)
	QSharedPointer<QAudioInput> audioInput();
#endif

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

private:
	ConferenceVideoWindow::Options _opts;                ///< Basic options for video conferencing.
	QSharedPointer<NetworkClient> _networkClient;        ///< Client for network communication.

	// Video stuff.
	QSharedPointer<QCamera> _camera;                     ///< The used camera for this conference.

#if defined(OCS_INCLUDE_AUDIO)
	// Audio stuff.
	QSharedPointer<QAudioInput> _audioInput;
	QSharedPointer<QAudioOutput> _audioOutput;
	QSharedPointer<AudioFramePlayer> _audioPlayer;
#endif

	// GUI stuff.
	ViewBase* _view;                                     ///< Central view to display all video streams.
};

#endif