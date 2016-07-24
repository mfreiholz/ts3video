#include "defaultstartuplogic.h"

#include <QMetaObject>
#include <QEventLoop>
#include <QHostAddress>

#include "videolib/elws.h"

#include "networkclient/networkclient.h"
#include "channellistwidget.h"

/*
- check if there is a dedicated server
	- by connecting to the same IP as given by "--address"
	- by looking up on ROUTE-MASTER and connecting to the result
- if connection to a deciated server has been established,
  show a dialog with public servers and the possibility to enter
  custom server information (address, port, password?)

Example arguments
	--mode default --server-address 127.0.0.1 --server-port 13370
*/

class DefaultStartupLogic::Private
{
public:
	//QHostAddress serverIp; /* resolved server IP (no dns!) */
};

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
	opts.serverAddress = ELWS::getArgsValue("--server-address").toString();
	opts.serverPort = ELWS::getArgsValue("--server-port").toString().toUInt();
	opts.serverPassword = ELWS::getArgsValue("--server-password").toString();

	// dns lookup for "opts.address"
	const auto serverIp = ELWS::resolveDns(opts.serverAddress);
	if (serverIp.isNull())
	{
		//emit error("can not resolve DNS");
		return;
	}

	//TODO check for dedicated server, by connecting to it.
	QSharedPointer<NetworkClient> nc(new NetworkClient());
	QObject::connect(nc.data(), &NetworkClient::connected, &loop, &QEventLoop::quit);
	QObject::connect(nc.data(), &NetworkClient::error, &loop, &QEventLoop::quit);
	nc->connectToHost(serverIp, opts.serverPort);
	loop.exec();
	if (nc->socket()->state() == QAbstractSocket::ConnectedState)
	{
		// show channel selection
		auto w = new ChannelListWidget(nc, nullptr);
		w->exec();
	}

	//TODO shall we
	//TODO get list of public servers
}