#include "cliententity.h"

#include <QStringList>

ClientEntity::ClientEntity() :
  id(0), name(), mediaAddress(), mediaPort(0)
{}

ClientEntity::ClientEntity(const ClientEntity &other)
{
  this->id = other.id;
  this->name = other.name;
}

ClientEntity::~ClientEntity()
{}

void ClientEntity::fromQJsonObject(const QJsonObject &obj)
{
  id = obj["id"].toInt();
  name = obj["name"].toString();
}

QJsonObject ClientEntity::toQJsonObject() const
{
  QJsonObject obj;
  obj["id"] = id;
  obj["name"] = name;
  return obj;
}

QString ClientEntity::toString() const
{
  QStringList sl;
  sl << QString::number(id) << name << mediaAddress << QString::number(mediaPort);
  return sl.join("#");
}