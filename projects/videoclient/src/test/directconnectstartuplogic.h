#ifndef DIRECTCONNECTSTARTUPLOGIC_H
#define DIRECTCONNECTSTARTUPLOGIC_H

#include <QObject>
#include <QSharedPointer>
#include "../startup/startuplogic.h"
#include "../networkclient/networkclient.h"

class DirectConnectStartupLogic : public QObject, public AbstractStartupLogic
{
	Q_OBJECT

public:
	DirectConnectStartupLogic(QApplication* a);
	virtual ~DirectConnectStartupLogic();
	virtual int exec();

private:
	QSharedPointer<NetworkClient> _nc;
};

#endif
