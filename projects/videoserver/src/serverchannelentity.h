#ifndef SERVERCHANNELENTITY_H
#define SERVERCHANNELENTITY_H

#include "channelentity.h"

class ServerChannelEntity : public ChannelEntity
{
public:
	ServerChannelEntity();
	ServerChannelEntity(const ServerChannelEntity& other);

public:
	QString ident; ///< The optional IDENT-String of this channel (alias)
	QString password; ///< Do not serialize this value.
};

#endif