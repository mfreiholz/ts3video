#include "defaultstartuplogic.h"
#include <QMetaObject>
#include <QEventLoop>
#include "videolib/elws.h"

/*
- check if there is a dedicated server
	- by connecting to the same IP as given by "--address"
	- by looking up on ROUTE-MASTER and connecting to the result
- if connection to a deciated server has been established,
  show a dialog with public servers and the possibility to enter
  custom server information (address, port, password?)
*/

DefaultStartupLogic::DefaultStartupLogic(QApplication* a) :
	AbstractStartupLogic(a)
{
}

DefaultStartupLogic::~DefaultStartupLogic()
{
}

int DefaultStartupLogic::exec()
{
	QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
	return AbstractStartupLogic::exec();
}

void DefaultStartupLogic::start()
{
	QEventLoop loop;

	// init startup parameters
	Options opts;
	opts.address = ELWS::getArgsValue("--address").toString();
	opts.port = ELWS::getArgsValue("--port").toString().toUInt();

}