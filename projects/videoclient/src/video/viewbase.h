#ifndef VIEWBASE_H
#define VIEWBASE_H

#include "baselib/defines.h"

#include "yuvframe.h"

class ClientEntity;
class ChannelEntity;

class ViewBase
{
public:
	ViewBase() {}
	virtual ~ViewBase() {};

	virtual void addClient(const ClientEntity& client, const ChannelEntity& channel) = 0;
	virtual void removeClient(const ClientEntity& client, const ChannelEntity& channel) = 0;
	virtual void updateClientVideo(YuvFrameRefPtr frame, ocs::clientid_t senderId) = 0;
};

#endif