#include "_legacy/modules/basemodule.h"

#include "QtCore/QStringList"

#include "_legacy/streamingserver.h"

BaseModule::BaseModule(StreamingServer *serverBase)
	: _serverBase(serverBase)
{
}

BaseModule::~BaseModule()
{
}

bool BaseModule::initialize()
{
	return true;
}

QStringList BaseModule::getMethodPrefixes() const
{
	return QStringList();
}

bool BaseModule::processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data)
{
	return false;
}

StreamingServer* BaseModule::getServerBase() const
{
	return _serverBase;
}
