#include "serverchannelentity.h"

ServerChannelEntity::ServerChannelEntity() :
	ChannelEntity()
{
}

ServerChannelEntity::ServerChannelEntity(const ServerChannelEntity& other) :
	ChannelEntity(other)
{
	ident = other.ident;
	password = other.password;
}

void ServerChannelEntity::merge(const ChannelEntity& other)
{
	ChannelEntity::merge(other);
}