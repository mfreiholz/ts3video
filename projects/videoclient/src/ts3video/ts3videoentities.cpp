#include "ts3videoentities.h"
#include <QJsonObject>
#include <QJsonValue>

bool VersionInfo::fromJson(const QJsonObject& json)
{
	version = json.value("version").toString();
	homepageUrl = json.value("homepageurl").toString();
	message = json.value("message").toString();
	releasedOn = json.value("releasedon").toString();
	return true;
}


bool ConferenceServerInfo::fromJson(const QJsonObject& json)
{
	address = json.value("address").toString();
	port = json.value("port").toInt();
	password = json.value("password").toString();
	return true;
}


bool ConferenceRoomInfo::fromJson(const QJsonObject& json)
{
	uid = json.value("uid").toString();
	password = json.value("password").toString();
	return true;
}


bool ConferenceJoinInfo::fromJson(const QJsonObject& json)
{
	if (!json.contains("server") || !server.fromJson(json.value("server").toObject()))
		return false;
	if (!json.contains("room") || !room.fromJson(json.value("room").toObject()))
		return false;
	return true;
}