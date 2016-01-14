#include "virtualserverconfigentity.h"
#include <QStringList>

VirtualServerConfigEntity::VirtualServerConfigEntity() :
	maxVideoResolutionWidth(0),
	maxVideoResolutionHeight(0),
	maxVideoBandwidth(0)
{
}

VirtualServerConfigEntity::VirtualServerConfigEntity(const VirtualServerConfigEntity& other)
{
	maxVideoResolutionWidth = other.maxVideoResolutionWidth;
	maxVideoResolutionHeight = other.maxVideoResolutionHeight;
	maxVideoBandwidth = other.maxVideoBandwidth;
}

VirtualServerConfigEntity& VirtualServerConfigEntity::operator=(const VirtualServerConfigEntity& other)
{
	maxVideoResolutionWidth = other.maxVideoResolutionWidth;
	maxVideoResolutionHeight = other.maxVideoResolutionHeight;
	maxVideoBandwidth = other.maxVideoBandwidth;
	return *this;
}

VirtualServerConfigEntity::~VirtualServerConfigEntity()
{
}

void VirtualServerConfigEntity::fromQJsonObject(const QJsonObject& obj)
{
	maxVideoResolutionWidth = obj["maxvideoresolutionwidth"].toInt();
	maxVideoResolutionHeight = obj["maxvideoresolutionheight"].toInt();
	maxVideoBandwidth = obj["maxvideobandwidth"].toInt();
}

QJsonObject VirtualServerConfigEntity::toQJsonObject() const
{
	QJsonObject obj;
	obj["maxvideoresolutionwidth"] = maxVideoResolutionWidth;
	obj["maxvideoresolutionheight"] = maxVideoResolutionHeight;
	obj["maxvideobandwidth"] = maxVideoBandwidth;
	return obj;
}

QString VirtualServerConfigEntity::toString() const
{
	QStringList sl;
	sl
		<< QString::number(maxVideoResolutionWidth)
		<< QString::number(maxVideoResolutionHeight)
		<< QString::number(maxVideoBandwidth)
		;
	return sl.join("#");
}