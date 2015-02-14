#ifndef CHANNELENTITY_H
#define CHANNELENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

class ChannelEntity
{
public:
  ChannelEntity();
  ChannelEntity(const ChannelEntity &other);
  ~ChannelEntity();
  void fromQJsonObject(const QJsonObject &obj);
  QJsonObject toQJsonObject() const;
  QString toString() const;

public:
  int id;
  QString name;
};
Q_DECLARE_METATYPE(ChannelEntity);
#endif