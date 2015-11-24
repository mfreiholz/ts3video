#ifndef TS3VIDEOSTARTUPLOGIC_H
#define TS3VIDEOSTARTUPLOGIC_H

#include <QDialog>
#include <QSharedPointer>

#include "startuplogic.h"
#include "video/conferencevideowindow.h"
#include "networkclient/networkclient.h"
#include "ts3video/ts3videoentities.h"

#include "ui_ts3videostartuplogic.h"


class Ts3VideoStartOptions
{
public:
	QString ts3ServerAddress;
	quint16 ts3ServerPort = 0;
	quint64 ts3ChannelId = 0;
	quint64 ts3ClientDbId = 0;
	QString ts3Username;

	// Indicates whether a public server should be used.
	bool usePublicConferenceServer = false;
};


class Ts3VideoStartupLogic : public QDialog, public AbstractStartupLogic
{
	Q_OBJECT
	Ui::TS3VideoStartupLogicDialogForm _ui;
	Ts3VideoStartOptions _args;
	ConferenceJoinInfo _joinInfo;
	QSharedPointer<NetworkClient> _nc;

public:
	Ts3VideoStartupLogic(QApplication* a);
	virtual ~Ts3VideoStartupLogic();
	virtual int exec();

private slots:
	void showProgress(const QString& text);
	void showResponseError(int status, const QString& errorMessage, const QString& details = QString());
	void showError(const QString& shortText, const QString& longText = QString());

private:
	void start();
	bool checkVersion();
	bool lookupConference();
	bool lookupPublicConference();
	void initNetwork();
	void authAndJoinConference();
	void startVideoGui();
	void quitDelayed();

private slots:
	void onConnected();
	void onDisconnected();
	void onError(QAbstractSocket::SocketError socketError);

signals:
	// Internal used signal, called from worker threads to update progress status.
	void newProgress(const QString& text);
};


#endif