#ifndef CONNECTIONTESTSTARTUPLOGIC_H
#define CONNECTIONTESTSTARTUPLOGIC_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
class QHostAddress;

#include "../startup/startuplogic.h"

class NetworkClient;

class ConnectionTestClient : public QObject
{
	Q_OBJECT
	friend class ConnectionTestStartupLogic;
	QSharedPointer<NetworkClient> _nc;
	bool _isMediaSocketAuthenticated;

public:
	ConnectionTestClient(const QHostAddress& serverAddress, const QString& user, QObject* parent);
	virtual ~ConnectionTestClient();

signals:
	void finished();
};

class ConnectionTestStartupLogic : public QObject, public AbstractStartupLogic
{
	Q_OBJECT

public:
	ConnectionTestStartupLogic(QApplication* a);
	virtual ~ConnectionTestStartupLogic();
	virtual int exec();
};

#endif