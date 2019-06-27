#pragma once
#include "httpserver/httprequesthandler.h"
class QJsonObject;
class QString;
class VirtualServer;

class HttpRouteHandler : public stefanfrings::HttpRequestHandler
{
	Q_OBJECT
public:
	HttpRouteHandler(VirtualServer* server = NULL);
	virtual ~HttpRouteHandler();
	virtual void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

protected:
	void done(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response, const QJsonObject& obj) const;
	void error(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response, const QString& error, int status = 400) const;
	void serviceGetChannels(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);
	void serviceCreateChannel(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

private:
	VirtualServer* _server;
};
