#include "serverchannelentity.h"

ServerChannelEntity::ServerChannelEntity() :
	ChannelEntity()
{
}

ServerChannelEntity::ServerChannelEntity(const ServerChannelEntity& other) :
	ChannelEntity(other)
{
	password = other.password;
}