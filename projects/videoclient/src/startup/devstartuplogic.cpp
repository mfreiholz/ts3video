#include "devstartuplogic.h"

#include "libapp/elws.h"

#include "libclient/networkclient/networkclient.h"

#include <QCameraInfo>
#include <QHostAddress>

#if defined(OCS_INCLUDE_AUDIO)
#include <QAudioDevicieInfo>
#endif

DevStartupLogic::DevStartupLogic(QApplication* a)
	: AbstractStartupLogic(a)
{
}

DevStartupLogic::~DevStartupLogic()
{
}

int DevStartupLogic::exec()
{
	// Init network.
	auto nc = QSharedPointer<NetworkClient>(new NetworkClient());
	nc->connectToHost(QHostAddress("127.0.0.1"), 13370);

	QObject::connect(nc.data(), &NetworkClient::connected, [this, nc]() {
		auto reply = nc->auth(ELWS::getUserName(), QString());
		QObject::connect(reply, &QCorReply::finished, [this, nc]() {
			auto reply = nc->joinChannelByIdentifier("default", QString());
			QObject::connect(reply, &QCorReply::finished, [this, nc]() {
				// Open conference video-chat UI.
				ConferenceVideoWindow::Options opts;
				opts.cameraDeviceId = QCameraInfo::defaultCamera().deviceName();
				opts.cameraAutoEnable = false;
#if defined(OCS_INCLUDE_AUDIO)
				opts.audioInputDeviceId = QAudioDeviceInfo::defaultInputDevice().deviceName();
				opts.audioInputAutoEnable = true;
#endif
				auto w = new ConferenceVideoWindow(opts, nc, nullptr, 0);
				w->show();
			});
			QCORREPLY_AUTODELETE(reply);
		});
		QCORREPLY_AUTODELETE(reply);
	});

	return AbstractStartupLogic::exec();
}