#include "devstartuplogic.h"
#include "videolib/src/elws.h"
#include "../networkclient/networkclient.h"
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QHostAddress>

DevStartupLogic::DevStartupLogic(QApplication* a) :
	AbstractStartupLogic(a)
{

}

DevStartupLogic::~DevStartupLogic()
{

}

int DevStartupLogic::exec()
{
	// Init network.
	auto nc = QSharedPointer<NetworkClient>(new NetworkClient());
	nc->connectToHost(QHostAddress("192.168.178.37"), 13370);

	QObject::connect(nc.data(), &NetworkClient::connected, [this, nc]()
	{
		auto reply = nc->auth(ELWS::getUserName(), QString());
		QObject::connect(reply, &QCorReply::finished, [this, nc]()
		{
			auto reply = nc->joinChannelByIdentifier("default", QString());
			QObject::connect(reply, &QCorReply::finished, [this, nc]()
			{
				// Open conference video-chat UI.
				ClientAppLogic::Options opts;
				opts.cameraDeviceId = QCameraInfo::defaultCamera().deviceName();
				opts.cameraAutoEnable = true;
				opts.audioInputDeviceId = QAudioDeviceInfo::defaultInputDevice().deviceName();
				opts.audioInputAutoEnable = true;
				auto w = new ClientAppLogic(opts, nc, nullptr, 0);
				w->show();
			});
			QCORREPLY_AUTODELETE(reply);
		});
		QCORREPLY_AUTODELETE(reply);
	});

	return AbstractStartupLogic::exec();
}