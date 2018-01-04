#include "cliententity.h"

#include <QStringList>

ClientEntity::ClientEntity() :
	id(0),
	name(),
	mediaAddress(),
	mediaPort(0),
	videoEnabled(false),
	videoWidth(0),
	videoHeight(0),
	videoBitrate(0),
	audioInputEnabled(false)
{
}

ClientEntity::ClientEntity(const ClientEntity& other)
{
	this->id = other.id;
	this->name = other.name;
	this->mediaAddress = other.mediaAddress;
	this->mediaPort = other.mediaPort;
	this->videoEnabled = other.videoEnabled;
	this->videoWidth = other.videoWidth;
	this->videoHeight = other.videoHeight;
	this->videoBitrate = other.videoBitrate;
	this->audioInputEnabled = other.audioInputEnabled;
}

ClientEntity& ClientEntity::operator = (const ClientEntity& other)
{
	this->id = other.id;
	this->name = other.name;
	this->mediaAddress = other.mediaAddress;
	this->mediaPort = other.mediaPort;
	this->videoEnabled = other.videoEnabled;
	this->videoWidth = other.videoWidth;
	this->videoHeight = other.videoHeight;
	this->videoBitrate = other.videoBitrate;
	this->audioInputEnabled = other.audioInputEnabled;
	return *this;
}

ClientEntity::~ClientEntity()
{}

void ClientEntity::fromQJsonObject(const QJsonObject& obj)
{
	id = obj["id"].toInt();
	name = obj["name"].toString();
	videoEnabled = obj["videoenabled"].toBool();
	videoWidth = obj["videowidth"].toInt();
	videoHeight = obj["videoheight"].toInt();
	videoBitrate = obj["videobitrate"].toInt();
	audioInputEnabled = obj["audioinputenabled"].toBool();
}

QJsonObject ClientEntity::toQJsonObject() const
{
	QJsonObject obj;
	obj["id"] = id;
	obj["name"] = name;
	obj["videoenabled"] = videoEnabled;
	obj["videowidth"] = videoWidth;
	obj["videoheight"] = videoHeight;
	obj["videobitrate"] = videoBitrate;
	obj["audioinputenabled"] = audioInputEnabled;
	return obj;
}

QString ClientEntity::toString() const
{
	QStringList sl;
	sl
			<< QString::number(id)
			<< name
			<< mediaAddress.toString()
			<< QString::number(mediaPort)
			<< QString::number(videoEnabled ? 1 : 0)
			<< QString::number(videoWidth)
			<< QString::number(videoHeight)
			<< QString::number(videoBitrate)
			<< QString::number(audioInputEnabled ? 1 : 0)
			;
	return sl.join("#");
}
