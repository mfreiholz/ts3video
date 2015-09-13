#ifndef TS3VIDEOSTARTUPLOGIC_H
#define TS3VIDEOSTARTUPLOGIC_H

#include <QDialog>
#include <QSharedPointer>

#include "../startuplogic.h"
#include "../clientapplogic.h"
#include "../networkclient/networkclient.h"

#include "ui_ts3videostartuplogic.h"


class Ts3VideoStartupLogic : public QDialog, public AbstractStartupLogic
{
	Q_OBJECT
	Ui::TS3VideoStartupLogicDialogForm _ui;
	ClientAppLogic::Options _opts;
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
	void lookupVideoServer();
	void initNetwork();
	void authAndJoinConference();
	void startVideoGui();
	void quitDelayed();

private slots:
	void onConnected();
	void onDisconnected();
	void onError(QAbstractSocket::SocketError socketError);

signals:
	void newProgress(const QString& text);
};


#endif