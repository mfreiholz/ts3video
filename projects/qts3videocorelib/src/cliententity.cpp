#include "cliententity.h"

#include <QStringList>

ClientEntity::ClientEntity() :
  id(0), name(), mediaAddress(), mediaPort(0), videoEnabled(false)
{}

ClientEntity::ClientEntity(const ClientEntity &other)
{
  this->id = other.id;
  this->name = other.name;
  this->mediaAddress = other.mediaAddress;
  this->mediaPort = other.mediaPort;
  this->videoEnabled = other.videoEnabled;
}

ClientEntity::~ClientEntity()
{}

void ClientEntity::fromQJsonObject(const QJsonObject &obj)
{
  id = obj["id"].toInt();
  name = obj["name"].toString();
  videoEnabled = obj["videoenabled"].toBool();
}

QJsonObject ClientEntity::toQJsonObject() const
{
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  obj["videoenabled"] = videoEnabled;
  return obj;
}

QString ClientEntity::toString() const
{
  QStringList sl;
  sl << QString::number(id) << name << mediaAddress << QString::number(mediaPort) << QString::number(videoEnabled ? 1 : 0);
  return sl.join("#");
}