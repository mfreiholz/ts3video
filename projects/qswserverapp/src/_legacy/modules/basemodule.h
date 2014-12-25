#ifndef BASEMODULE_HEADER
#define BASEMODULE_HEADER

#include "shared/network/tcp_controlling_protocol.h"

class QVariant;
class QStringList;
class StreamingServer;
class TcpSocketHandler;

class BaseModule
{
public:
	BaseModule(StreamingServer *serverBase);
	virtual ~BaseModule();

	/*
		This function gets called from server base on startup,
		before any requests from clients are handled.

		\return Returns true, if the module has been initialized and false,
			any kind of error occured during initialization. The module will
			not be used in case of error.
	*/
	virtual bool initialize();

	/*
		Gets a list of methods which the module can handle in its
		processJsonRequest() method.
	*/
	virtual QStringList getMethodPrefixes() const;

	/*
		Processes a JSON requests from a client.
	*/
	virtual bool processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);

protected:
	StreamingServer* getServerBase() const;

protected:
	StreamingServer *_serverBase;
};

#endif
