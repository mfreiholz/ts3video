#ifndef CHANNELENTITY_H
#define CHANNELENTITY_H

#include <QString>
#include <QJsonObject>

class ChannelEntity
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