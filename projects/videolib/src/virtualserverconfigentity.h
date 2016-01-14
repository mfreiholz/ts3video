#ifndef VIRTUALSERVERCONFIGENTITY_H
#define VIRTUALSERVERCONFIGENTITY_H

#include <QString>
#include <QJsonObject>

/*
	This object contains all properties which are public for all clients.
*/
class VirtualServerConfigEntity
{
public:
	VirtualServerConfigEntity();
	VirtualServerConfigEntity(const VirtualServerConfigEntity& other);
	VirtualServerConfigEntity& operator=(const VirtualServerConfigEntity& other);
	~VirtualServerConfigEntity();

	void fromQJsonObject(const QJsonObject& obj);
	QJsonObject toQJsonObject() const;
	QString toString() const;

public:
	// Video stuff
	int maxVideoResolutionWidth;
	int maxVideoResolutionHeight;
	int maxVideoBandwidth;
};

#endif