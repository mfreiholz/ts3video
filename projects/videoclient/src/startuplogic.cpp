#include "startuplogic.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QHash>
#include <QHashIterator>

#include "humblelogging/api.h"

#include "videolib/src/ts3video.h"
#include "videolib/src/elws.h"

///////////////////////////////////////////////////////////////////////

AbstractStartupLogic::AbstractStartupLogic(QApplication* a) :
	_qapp(a)
{
	a->setOrganizationName("insaneFactory");
	a->setOrganizationDomain("http://ts3video.insanefactory.com/");
	a->setApplicationName("Video Client");
	a->setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);

#ifdef _WIN32
	// Show console window?
	if (ELWS::hasArgsValue("--console"))
	{
		AllocConsole();
	}
#endif

	initLogging();
	initStyleSheet();
}

AbstractStartupLogic::~AbstractStartupLogic()
{
	_qapp = nullptr;
}

QApplication* AbstractStartupLogic::qapp() const
{
	return _qapp;
}

QString AbstractStartupLogic::configFilePath() const
{
	return ELWS::getArgsValue("--config", _qapp->applicationDirPath() + QString("/default.ini")).toString();
}

int AbstractStartupLogic::exec()
{
	return _qapp->exec();
}

void AbstractStartupLogic::initLogging()
{
	QHash<QString, QString> env;
	env.insert("$APPDIR", _qapp->applicationDirPath());
	env.insert("$TEMPDIR", QDir::tempPath());

	QSettings conf(configFilePath());
	conf.beginGroup("logging");
	auto logFilePath = conf.value("FilePath", QString("$TEMPDIR/clientapp.log")).toString();
	auto logConfigFilePath = conf.value("ConfigFilePath", QString("$APPDIR/logging.conf")).toString();
	auto consoleLoggerEnabled = conf.value("ConsoleAppenderEnabled", 0).toInt() == 1;
	conf.endGroup();

	QHashIterator<QString, QString> itr(env);
	while (itr.hasNext())
	{
		itr.next();
		logFilePath = logFilePath.replace(itr.key(), itr.value(), Qt::CaseSensitive);
		logConfigFilePath = logConfigFilePath.replace(itr.key(), itr.value(), Qt::CaseSensitive);
	}

	auto& fac = humble::logging::Factory::getInstance();
	fac.setDefaultFormatter(new humble::logging::PatternFormatter("%date\t%lls\ttid=%tid\t%m\n"));
	if (true)
		fac.registerAppender(new humble::logging::FileAppender(logFilePath.toStdString(), true));
	if (consoleLoggerEnabled)
		fac.registerAppender(new humble::logging::ConsoleAppender());
	fac.changeGlobalLogLevel(humble::logging::LogLevel::Debug);
}

void AbstractStartupLogic::initStyleSheet()
{
	if (ELWS::hasArgsValue("--no-style"))
		return;
	QFile f(":/default.css");
	f.open(QFile::ReadOnly);
	_qapp->setStyleSheet(QString(f.readAll()));
	f.close();
}
