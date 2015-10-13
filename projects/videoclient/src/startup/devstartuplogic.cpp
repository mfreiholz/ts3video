#include "devstartuplogic.h"
#include "../networkclient/networkclient.h"
#include <QCameraInfo>

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

	// Open conference video-chat UI.
	ClientAppLogic::Options opts;
	opts.cameraDeviceId = QCameraInfo::defaultCamera().deviceName();
	opts.cameraAutoEnable = true;
	auto w = new ClientAppLogic(opts, nc, nullptr, 0);
	w->show();

	return AbstractStartupLogic::exec();
}