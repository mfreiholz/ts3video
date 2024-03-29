#include "virtualserverconfigentity.h"
#include <QStringList>
#include <QSize>

VirtualServerConfigEntity::VirtualServerConfigEntity() :
	maxVideoResolutionWidth(0),
	maxVideoResolutionHeight(0),
	maxVideoBitrate(0)
{
}

VirtualServerConfigEntity::VirtualServerConfigEntity(const VirtualServerConfigEntity& other)
{
	maxVideoResolutionWidth = other.maxVideoResolutionWidth;
	maxVideoResolutionHeight = other.maxVideoResolutionHeight;
	maxVideoBitrate = other.maxVideoBitrate;
}

VirtualServerConfigEntity& VirtualServerConfigEntity::operator=(const VirtualServerConfigEntity& other)
{
	maxVideoResolutionWidth = other.maxVideoResolutionWidth;
	maxVideoResolutionHeight = other.maxVideoResolutionHeight;
	maxVideoBitrate = other.maxVideoBitrate;
	return *this;
}

VirtualServerConfigEntity::~VirtualServerConfigEntity()
{
}

void VirtualServerConfigEntity::fromQJsonObject(const QJsonObject& obj)
{
	maxVideoResolutionWidth = obj["maxvideoresolutionwidth"].toInt();
	maxVideoResolutionHeight = obj["maxvideoresolutionheight"].toInt();
	maxVideoBitrate = obj["maxvideobitrate"].toInt();
}

QJsonObject VirtualServerConfigEntity::toQJsonObject() const
{
	QJsonObject obj;
	obj["maxvideoresolutionwidth"] = maxVideoResolutionWidth;
	obj["maxvideoresolutionheight"] = maxVideoResolutionHeight;
	obj["maxvideobitrate"] = maxVideoBitrate;
	return obj;
}

QString VirtualServerConfigEntity::toString() const
{
	QStringList sl;
	sl
			<< QString::number(maxVideoResolutionWidth)
			<< QString::number(maxVideoResolutionHeight)
			<< QString::number(maxVideoBitrate)
			;
	return sl.join("#");
}

bool VirtualServerConfigEntity::isResolutionSupported(const VirtualServerConfigEntity& config, const QSize& size)
{
	if (size.width() > config.maxVideoResolutionWidth || size.height() > config.maxVideoResolutionHeight)
		return false;
	return true;
}

bool VirtualServerConfigEntity::isBitrateSupported(const VirtualServerConfigEntity& config, int bitrate)
{
	if (bitrate > config.maxVideoBitrate)
		return false;
	return true;
}