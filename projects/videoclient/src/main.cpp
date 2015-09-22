#ifdef _WIN32
#include <Windows.h>
#endif

#include <QObject>
#include <QTimer>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QCamera>
#include <QCameraInfo>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QDir>

#include "humblelogging/api.h"

#include "qcorreply.h"

#include "ts3video.h"
#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"

#include "startuplogic.h"
#include "startup/ts3videostartuplogic.h"

#include "cameraframegrabber.h"
#include "videowidget.h"
#include "videocollectionwidget.h"
#include "networkclient/networkclient.h"
#include "clientapplogic.h"
#include "startupwidget.h"
#include "tileviewwidget.h"

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