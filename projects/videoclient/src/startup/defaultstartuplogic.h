#pragma once

#include <QObject>
#include "startuplogic.h"

class DefaultStartupLogic :
	public QObject,
	public AbstractStartupLogic
{
	Q_OBJECT

public:
	class Options
	{
	public:
		QString address;
		quint16 port;
		QString displayName;
	};

	explicit DefaultStartupLogic(QApplication* a);
	virtual ~DefaultStartupLogic();
	virtual int exec();

private slots:
	void start();
};