#pragma once

#include <memory>
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
		QString serverAddress;
		quint16 serverPort;
		QString serverPassword;
		QString userDisplayName;
	};

	explicit DefaultStartupLogic(QApplication* a);
	virtual ~DefaultStartupLogic();
	virtual int exec();

private slots:
	void start();

private:
	class Private;
	std::unique_ptr<Private> d;
};