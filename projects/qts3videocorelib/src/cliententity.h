#ifndef CLIENTENTITY_H
#define CLIENTENTITY_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QMetaType>

class ClientEntity
{
public:
  ClientEntity() :
    id(0), name(), mediaAddress(), mediaPort(0)
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

  QString toString() const
  {
    QStringList sl;
    sl << QString::number(id) << name << mediaAddress << QString::number(mediaPort);
    return sl.join("#");
  }

public:
  int id;
  QString name;
  QString mediaAddress; ///< Do not serialize, as long as we don't support P2P streaming.
  quint16 mediaPort; ///< Do not serialize, as long as we don't support P2P streaming.
};
Q_DECLARE_METATYPE(ClientEntity);
#endif