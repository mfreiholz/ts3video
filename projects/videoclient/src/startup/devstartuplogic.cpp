#include "devstartuplogic.h"

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
	auto w = new ClientAppLogic(opts, nc, nullptr, 0);
	w->show();

	return AbstractStartupLogic::exec();
}