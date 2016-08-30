#ifdef _WIN32
#include <Windows.h>
#endif
#include <QApplication>
#include <QScopedPointer>

#include "humblelogging/api.h"

#include "libbase/defines.h"

#include "videolib/ts3video.h"
#include "videolib/elws.h"

#include "startup/devstartuplogic.h"
#include "startup/defaultstartuplogic.h"
#include "ts3video/ts3videostartuplogic.h"
#include "test/connectionteststartuplogic.h"
#include "test/directconnectstartuplogic.h"

HUMBLE_LOGGER(HL, "client");

/*
	Runs a implementation of AbstractStartupLogic based on command line
	parameter "--mode".
*/
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	qRegisterMetaType<ocs::clientid_t>("ocs::clientid_t");
	qRegisterMetaType<ocs::channelid_t>("ocs::channelid_t");

#ifdef _WIN32
	// Show console window?
	if (ELWS::hasArgsValue("--console"))
	{
		AllocConsole();
	}
#endif

	// Startup with correct mode (logic).
	QScopedPointer<AbstractStartupLogic> sl;
	const auto mode = ELWS::getArgsValue("--mode").toString();
	if (mode.compare("ts3video", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new Ts3VideoStartupLogic(&a));
	}
	else if (mode.compare("dev", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new DevStartupLogic(&a));
	}
	else if (mode.compare("connection-test", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new ConnectionTestStartupLogic(&a));
	}
	else if (mode.compare("directconnect", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new DirectConnectStartupLogic(&a));
	}
	else if (mode.compare("default", Qt::CaseInsensitive) == 0)
	{
		sl.reset(new DefaultStartupLogic(&a));
	}
	if (!sl)
	{
		return 500; // No startup logic.
	}
	auto returnCode = sl->exec();
	HL_INFO(HL, QString("Application exit (code=%1)").arg(returnCode).toStdString());
	return returnCode;
}