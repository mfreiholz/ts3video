#ifndef SERVERCHANNELENTITY_H
#define SERVERCHANNELENTITY_H

#include "channelentity.h"

class ServerChannelEntity : public ChannelEntity
{
public:
	ServerChannelEntity();
	ServerChannelEntity(const ServerChannelEntity& other);

public:
	QString password; ///< Do not serialize this value.
};

#endif