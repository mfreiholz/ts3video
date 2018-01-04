#ifndef SERVERCHANNELENTITY_H
#define SERVERCHANNELENTITY_H

#include "libapp/channelentity.h"

class ServerChannelEntity : public ChannelEntity
{
public:
	ServerChannelEntity();
	ServerChannelEntity(const ServerChannelEntity& other);

	void merge(const ChannelEntity& c);

public:
	QString ident; ///< The optional IDENT-String of this channel (alias)
	QString password; ///< Do not serialize this value.
};

#endif