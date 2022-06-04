#include "App.hpp"
#include "RemoteVideoAdapter.hpp"
#include "libbase/defines.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char** argv)
{
	QGuiApplication qtGuiApp(argc, argv);
	qRegisterMetaType<ocs::clientid_t>("ocs::clientid_t");
	qRegisterMetaType<ocs::channelid_t>("ocs::channelid_t");

	App app(nullptr);
	RemoteVideoAdapter::registerQmlTypes();

	QQmlApplicationEngine qmlEngine;
	qmlEngine.rootContext()->setContextProperty("app", &app);
	qmlEngine.load(QUrl(QStringLiteral("qrc:/qml/App.qml")));
	if (qmlEngine.rootObjects().isEmpty())
		return -1;
	return qtGuiApp.exec();
}