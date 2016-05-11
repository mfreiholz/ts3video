#include <QHostAddress>
#include <QTimer>
#include <QTime>
#include "connectionteststartuplogic.h"
#include "../networkclient/networkclient.h"

///////////////////////////////////////////////////////////////////////////////

ConnectionTestClient::ConnectionTestClient(const QHostAddress& serverAddress, const QString& user, QObject* parent) :
	QObject(parent),
	_isMediaSocketAuthenticated(false)
{
	_nc = QSharedPointer<NetworkClient>(new NetworkClient());
	_nc->connectToHost(serverAddress, 13370);

	QObject::connect(_nc.data(), &NetworkClient::connected, [this, user]()
	{
		auto reply = _nc->auth(user, "Secret!!!");
		QCorReply::autoDelete(reply);
	});

	QObject::connect(_nc.data(), &NetworkClient::mediaSocketAuthenticated, [this]()
	{
		_isMediaSocketAuthenticated = true;
		auto reply = _nc->joinChannelByIdentifier("default", QString());
		QObject::connect(reply, &QCorReply::finished, [reply]()
		{
			reply->frame();
		});
		QCorReply::autoDelete(reply);
	});

	auto closeTimer = new QTimer(this);
	closeTimer->setSingleShot(true);
	QObject::connect(closeTimer, &QTimer::timeout, [this]()
	{
		if (!_isMediaSocketAuthenticated)
		{
			qWarning() << "oh oh...";
			return;
		}
		auto reply = _nc->goodbye();
		QCorReply::autoDelete(reply);
		QObject::connect(reply, &QCorReply::finished, this, &ConnectionTestClient::deleteLater);
	});
	closeTimer->start(3000);
}

ConnectionTestClient::~ConnectionTestClient()
{
}

///////////////////////////////////////////////////////////////////////////////

ConnectionTestStartupLogic::ConnectionTestStartupLogic(QApplication* a) :
	AbstractStartupLogic(a)
{
}

ConnectionTestStartupLogic::~ConnectionTestStartupLogic()
{
}

int ConnectionTestStartupLogic::exec()
{
	QTimer timer;
	timer.setInterval(250);
	timer.setSingleShot(true);
	QObject::connect(&timer, &QTimer::timeout, [this]()
	{
		auto c = new ConnectionTestClient(QHostAddress("81.169.176.229"), "Test User", this);
	});
	timer.start();

	return AbstractStartupLogic::exec();
}