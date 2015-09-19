#ifndef TS3VIDEOENTITIES_H
#define TS3VIDEOENTITIES_H

#include <QString>
class QJsonObject;


class VersionInfo
{
public:
	QString version;
	QString homepageUrl;
	QString message;
	QString releasedOn;

	bool fromJson(const QJsonObject& json);
};


class ConferenceServerInfo
{
public:
	QString address;
	quint16 port;
	QString password;

	bool fromJson(const QJsonObject& json);
};


class ConferenceRoomInfo
{
public:
	QString uid;
	QString password;

	bool fromJson(const QJsonObject& json);
};

class ConferenceJoinInfo
{
public:
	ConferenceServerInfo server;
	ConferenceRoomInfo room;

	bool fromJson(const QJsonObject& json);
};


#endif