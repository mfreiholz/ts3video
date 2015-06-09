#ifndef CLIENTLISTMODELPRIVATE_H
#define CLIENTLISTMODELPRIVATE_H

#include "clientlistmodel.h"

#include "cliententity.h"

class ClientListModelPrivate : public QObject
{
  Q_OBJECT

public:
  ClientListModelPrivate(ClientListModel *o);
  int findIndexOfClient(const ClientEntity &c) const;

public slots:
  void onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void onClientDisconnected(const ClientEntity &client);

public:
  ClientListModel *owner;
  NetworkClient *networkClient;
  QList<ClientEntity>  clients;
};

#endif