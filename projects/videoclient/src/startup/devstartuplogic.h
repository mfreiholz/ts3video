#ifndef DEVSTARTUPLOGIC_H
#define DEVSTARTUPLOGIC_H

#include <QObject>
#include "startuplogic.h"
#include "video/conferencevideowindow.h"

class DevStartupLogic : public QObject, public AbstractStartupLogic
{
	Q_OBJECT

public:
	DevStartupLogic(QApplication* a);
	virtual ~DevStartupLogic();
	virtual int exec();
};

#endif