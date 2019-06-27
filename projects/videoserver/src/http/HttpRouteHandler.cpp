#include "HttpRouteHandler.h"
#include "../virtualserver.h"
#include "../serverchannelentity.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

HttpRouteHandler::HttpRouteHandler(VirtualServer* server) :
	stefanfrings::HttpRequestHandler(server),
	_server(server)
{}

HttpRouteHandler::~HttpRouteHandler()
{
	_server = NULL;
}

void
HttpRouteHandler::service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response)
{
	if (request.getMethod() == "GET" && request.getPath() == "/api/1.0/channels")
	{
		serviceGetChannels(request, response);
	}
	else if (request.getMethod() == "POST" && request.getPath() == "/api/1.0/channels")
	{
		serviceCreateChannel(request, response);
	}
	else
	{
		error(request, response, "Not Found", 404);
	}
}

void
HttpRouteHandler::done(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response, const QJsonObject& obj) const
{
	response.setStatus(200);
	response.setHeader("Content-Type", "application/json; charset=UTF-8");
	response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void
HttpRouteHandler::error(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response, const QString& error, int status) const
{
	QJsonObject obj;
	obj.insert("error", error);
	response.setStatus(status);
	response.setHeader("Content-Type", "application/json; charset=UTF-8");
	response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void
HttpRouteHandler::serviceGetChannels(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response)
{
	QMutexLocker l(&_server->_mutex);
	auto channels = _server->_id2channel.values();
	l.unlock();

	QJsonObject jroot;
	QJsonArray jchannels;
	for (auto channel : channels)
	{
		jchannels.append(channel->toQJsonObject());
	}
	jroot.insert("channels", jchannels);

	done(request, response, jroot);
}

void
HttpRouteHandler::serviceCreateChannel(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response)
{
}
