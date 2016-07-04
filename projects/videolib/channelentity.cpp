#include "channelentity.h"

#include <QStringList>

ChannelEntity::ChannelEntity() :
	id(0),
	isPasswordProtected(false),
	isPersistent(false)
{
}

ChannelEntity::ChannelEntity(const ChannelEntity& other)
{
	this->id = other.id;
	this->name = other.id;
	this->isPasswordProtected = other.isPasswordProtected;
	this->isPersistent = other.isPersistent;
}

ChannelEntity& ChannelEntity::operator=(const ChannelEntity& other)
{
	this->id = other.id;
	this->name = other.id;
	this->isPasswordProtected = other.isPasswordProtected;
	this->isPersistent = other.isPersistent;
	return *this;
}

ChannelEntity::~ChannelEntity()
{
}

void ChannelEntity::fromQJsonObject(const QJsonObject& obj)
{
	id = obj["id"].toInt();
	name = obj["name"].toString();
	isPasswordProtected = obj["ispasswordprotected"].toBool();
	isPersistent = obj["ispersistent"].toBool();
}

QJsonObject ChannelEntity::toQJsonObject() const
{
	QJsonObject obj;
	obj["id"] = id;
	obj["name"] = name;
	obj["ispasswordprotected"] = isPasswordProtected;
	obj["ispersistent"] = isPersistent;
	return obj;
}

QString ChannelEntity::toString() const
{
	QStringList sl;
	sl
			<< QString::number(id)
			<< name
			<< (isPasswordProtected ? QString("true") : QString("false"))
			<< (isPersistent ? QString("true") : QString("false"))
			;
	return sl.join("#");
}
