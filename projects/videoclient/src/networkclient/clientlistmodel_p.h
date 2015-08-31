#ifndef CLIENTLISTMODELPRIVATE_H
#define CLIENTLISTMODELPRIVATE_H

#include "clientlistmodel.h"

#include "cliententity.h"
#include "channelentity.h"

class ClientListModelPrivate : public QObject
{
	Q_OBJECT

public:
	ClientListModelPrivate(ClientListModel* o);
	int findIndexOfClient(const ClientEntity& c) const;

public slots:
	void onClientJoinedChannel(const ClientEntity& client, const ChannelEntity& channel);
	void onClientLeftChannel(const ClientEntity& client, const ChannelEntity& channel);
	void onClientDisconnected(const ClientEntity& client);
	void onClientEnabledVideo(const ClientEntity& client);
	void onClientDisabledVideo(const ClientEntity& client);

public:
	ClientListModel* owner;
	NetworkClient* networkClient;
	ChannelEntity channel;
	QList<ClientEntity>  clients;
};

#endif