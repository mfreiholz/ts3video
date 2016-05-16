#include <QHostAddress>
#include <QCameraInfo>
#include "directconnectstartuplogic.h"
#include "../video/conferencevideowindow.h"

DirectConnectStartupLogic::DirectConnectStartupLogic(QApplication* a) :
	AbstractStartupLogic(a),
	QObject(nullptr)
{
}

DirectConnectStartupLogic::~DirectConnectStartupLogic()
{
}

int DirectConnectStartupLogic::exec()
{
	_nc = QSharedPointer<NetworkClient>(new NetworkClient());
	_nc->connectToHost(QHostAddress("81.169.137.183"), 13370);

	QObject::connect(_nc.data(), &NetworkClient::connected, [this]()
	{
		auto reply = _nc->auth("testuser", QString());
		QCorReply::autoDelete(reply);
	});

	QObject::connect(_nc.data(), &NetworkClient::mediaSocketAuthenticated, [this]()
	{
		auto reply = _nc->joinChannelByIdentifier("default", QString());
		QObject::connect(reply, &QCorReply::finished, [this, reply]()
		{
			ConferenceVideoWindow::Options opts;
			opts.cameraDeviceId = QCameraInfo::defaultCamera().deviceName();
			opts.cameraAutoEnable = true;
			auto w = new ConferenceVideoWindow(_nc, nullptr, 0);
			w->applyOptions(opts);
			w->show();
		});
		QCorReply::autoDelete(reply);
	});

	return AbstractStartupLogic::exec();
}
