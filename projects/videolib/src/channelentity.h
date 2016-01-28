#ifndef CHANNELENTITY_H
#define CHANNELENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

#include "baselib/defines.h"

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
  ocs::channelid_t id;
  QString name;
  bool isPasswordProtected;
};
Q_DECLARE_METATYPE(ChannelEntity);
#endif