#ifndef CLIENTLISTMODELPRIVATE_H
#define CLIENTLISTMODELPRIVATE_H

#include "clientlistmodel.h"

#include "cliententity.h"

class ClientListModelPrivate : public QObject
{
  Q_OBJECT

public:
  ClientListModelPrivate(ClientListModel *o) :
    QObject(o),
    owner(o)
  {}

public:
  int findIndexOfClient(const ClientEntity &c) const;

public:
  ClientListModel *owner;
  QList<ClientEntity>  clients;
};

#endif