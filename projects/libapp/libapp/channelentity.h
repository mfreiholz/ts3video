#ifndef CHANNELENTITY_H
#define CHANNELENTITY_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

#include "libbase/defines.h"

class ChannelEntity
{
public:
	ChannelEntity();
	ChannelEntity(const ChannelEntity& other);
	ChannelEntity& operator=(const ChannelEntity& other);
	~ChannelEntity();

	// merges all attributes except the ID.
	void merge(const ChannelEntity& other);

	void fromQJsonObject(const QJsonObject& obj);
	QJsonObject toQJsonObject() const;
	QString toString() const;

public:
	ocs::channelid_t id;
	QString name;
	bool isPasswordProtected;
	bool isPersistent;
};
Q_DECLARE_METATYPE(ChannelEntity);
#endif
