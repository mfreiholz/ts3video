#include "channelentity.h"

#include <QStringList>

ChannelEntity::ChannelEntity() :
  id(0),
  isPasswordProtected(false)
{}

ChannelEntity::ChannelEntity(const ChannelEntity &other)
{
  this->id = other.id;
  this->name = other.id;
  this->isPasswordProtected = other.isPasswordProtected;
}

ChannelEntity::~ChannelEntity()
{
}

void ChannelEntity::fromQJsonObject(const QJsonObject &obj)
{
  id = obj["id"].toInt();
  name = obj["name"].toString();
  isPasswordProtected = obj["ispasswordprotected"].toBool();
}

QJsonObject ChannelEntity::toQJsonObject() const
{
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  obj["ispasswordprotected"] = isPasswordProtected;
  return obj;
}

QString ChannelEntity::toString() const
{
  QStringList sl;
  sl << QString::number(id) << name << (isPasswordProtected ? QString("true") : QString("false"));
  return sl.join("#");
}