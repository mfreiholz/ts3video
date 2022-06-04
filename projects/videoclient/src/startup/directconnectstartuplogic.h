#ifndef DIRECTCONNECTSTARTUPLOGIC_H
#define DIRECTCONNECTSTARTUPLOGIC_H

#include "../startup/startuplogic.h"
#include "libclient/networkclient/networkclient.h"
#include <QObject>
#include <QSharedPointer>

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
