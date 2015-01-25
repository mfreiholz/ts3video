#ifndef CLIENTENTITY_H
#define CLIENTENTITY_H

#include <QString>
#include <QJsonObject>

class ClientEntity
{
public:
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

#endif