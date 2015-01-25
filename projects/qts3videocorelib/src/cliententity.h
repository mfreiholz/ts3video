#ifndef CLIENTENTITY_H
#define CLIENTENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

class ClientEntity
{
public:
  ClientEntity()
  {}

  ClientEntity(const ClientEntity &other)
  {
    this->id = other.id;
    this->name = other.name;
  }

  ~ClientEntity()
  {}

  void fromQJsonObject(const QJsonObject &obj)
  {
    id = obj["id"].toInt();
    name = obj["name"].toString();
  }

  QJsonObject toQJsonObject() const
  {
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    return obj;
  }

public:
  int id;
  QString name;
};
Q_DECLARE_METATYPE(ClientEntity);
#endif