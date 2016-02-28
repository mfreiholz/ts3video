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
#include <QString>
#include <QObject>

#include "humblelogging/api.h"

#include "videolib/src/ts3video.h"
#include "videolib/src/elws.h"

///////////////////////////////////////////////////////////////////////

AbstractStartupLogic::AbstractStartupLogic(QApplication* a) :
	_qapp(a)
{
	initApplication();
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

void AbstractStartupLogic::initApplication()
{
	_qapp->setOrganizationDomain("https://mfreiholz.de");
	_qapp->setApplicationName("Conference Client");
	_qapp->setApplicationDisplayName(QObject::tr("Conference Client"));
	_qapp->setApplicationVersion(IFVS_SOFTWARE_VERSION_QSTRING);

#ifdef _WIN32
	// Show console window?
	if (ELWS::hasArgsValue("--console"))
	{
		AllocConsole();
	}
#endif
}

void AbstractStartupLogic::initLogging()
{
	QHash<QString, QString> env;
	env.insert("$APPDIR", _qapp->applicationDirPath());
	env.insert("$TEMPDIR", QDir::tempPath());

	QSettings conf(configFilePath(), QSettings::IniFormat);
	conf.beginGroup("logging");
	auto logFilePath = conf.value("FilePath", "$TEMPDIR/conference-client.log").toString();
	auto logConfigFilePath = conf.value("ConfigFilePath", "$APPDIR/logging.conf").toString();
	auto consoleLoggerEnabled = conf.value("ConsoleAppenderEnabled", false).toBool();
	conf.endGroup();

	QHashIterator<QString, QString> itr(env);
	while (itr.hasNext())
	{
		itr.next();
		logFilePath = logFilePath.replace(itr.key(), itr.value(), Qt::CaseSensitive);
		logConfigFilePath = logConfigFilePath.replace(itr.key(), itr.value(), Qt::CaseSensitive);
	}

	auto& fac = humble::logging::Factory::getInstance();
	if (logConfigFilePath.isEmpty() || !QFile::exists(logConfigFilePath))
		fac.setConfiguration(new humble::logging::SimpleConfiguration(humble::logging::LogLevel::Info));
	else
		fac.setConfiguration(humble::logging::DefaultConfiguration::createFromFile(logConfigFilePath.toStdString()));
	fac.setDefaultFormatter(new humble::logging::PatternFormatter("%date\t%lls\tpid=%pid\ttid=%tid\t%m\n"));
	if (!logFilePath.isEmpty())
		fac.registerAppender(new humble::logging::FileAppender(logFilePath.toStdString(), true));
	if (consoleLoggerEnabled)
		fac.registerAppender(new humble::logging::ConsoleAppender());
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
