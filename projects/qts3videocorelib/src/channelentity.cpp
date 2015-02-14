#include "channelentity.h"

#include <QStringList>

ChannelEntity::ChannelEntity()
{}

ChannelEntity::ChannelEntity(const ChannelEntity &other)
{
  this->id = other.id;
  this->name = other.id;
}

ChannelEntity::~ChannelEntity()
{
}

void ChannelEntity::fromQJsonObject(const QJsonObject &obj)
{
  id = obj["id"].toInt();
  name = obj["name"].toString();
}

QJsonObject ChannelEntity::toQJsonObject() const
{
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  return obj;
}

QString ChannelEntity::toString() const
{
  QStringList sl;
  sl << QString::number(id) << name;
  return sl.join("#");
}