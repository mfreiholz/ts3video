#ifndef VIEWBASE_H
#define VIEWBASE_H

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
	virtual void updateClientVideo(YuvFrameRefPtr frame, int senderId) = 0;
};

#endif