#ifdef _WIN32
#include <Windows.h>
#endif
#include <QApplication>
#include <QScopedPointer>
#include "humblelogging/api.h"
#include "videolib/src/ts3video.h"
#include "videolib/src/elws.h"
#include "startup/ts3videostartuplogic.h"

HUMBLE_LOGGER(HL, "client");

/*
	Runs a implementation of AbstractStartupLogic based on command line
	parameter "--mode".
*/
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

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
	if (!sl)
	{
		return 500; // No startup logic.
	}
	auto returnCode = sl->exec();
	HL_INFO(HL, QString("Application exit (code=%1)").arg(returnCode).toStdString());
	return returnCode;
}