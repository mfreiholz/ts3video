#include "clientlistmodel_p.h"

#include <QIcon>

#include "../networkclient/networkclient.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

ClientListModel::ClientListModel(QObject *parent) :
  QAbstractListModel(parent),
  d(new ClientListModelPrivate(this))
{
}

ClientListModel::~ClientListModel()
{
}

void ClientListModel::setNetworkClient(NetworkClient *networkClient)
{
  beginResetModel();
  if (d->networkClient)
  {
    d->networkClient->disconnect(d.data());
    d->networkClient->disconnect(this);
  }
  d->networkClient = networkClient;
  connect(d->networkClient, &NetworkClient::clientJoinedChannel, d.data(), &ClientListModelPrivate::onClientJoinedChannel);
  connect(d->networkClient, &NetworkClient::clientLeftChannel, d.data(), &ClientListModelPrivate::onClientLeftChannel);
  connect(d->networkClient, &NetworkClient::clientDisconnected, d.data(), &ClientListModelPrivate::onClientDisconnected);
  d->clients.clear();
  endResetModel();
}

void ClientListModel::addClient(const ClientEntity &client)
{
  beginInsertRows(QModelIndex(), d->clients.size(), d->clients.size());
  d->clients.append(client);
  endInsertRows();
}

void ClientListModel::removeClient(const ClientEntity &client)
{
  auto index = d->findIndexOfClient(client);
  if (index == -1)
    return;
  beginRemoveRows(QModelIndex(), index, index);
  d->clients.removeAt(index);
  endRemoveRows();
}

int ClientListModel::rowCount(const QModelIndex &parent) const
{
  return d->clients.size();
}

QVariant ClientListModel::data(const QModelIndex &index, int role) const
{
  if (index.row() > d->clients.size() - 1)
    return QVariant();

  auto &client = d->clients[index.row()];

  switch (role)
  {
  case Qt::DisplayRole:
    return client.name;

  case Qt::DecorationRole:
    if (client.videoEnabled)
      return QIcon(":/ic_videocam_grey600_48dp.png");
    else
      return QIcon(":/ic_videocam_off_grey600_48dp.png");

  case VideoEnabledRole:
    return client.videoEnabled;

  case ClientEntityRole:
    return QVariant::fromValue(client);
  }

  return QVariant();
}

///////////////////////////////////////////////////////////////////////

ClientListModelPrivate::ClientListModelPrivate(ClientListModel *o) :
  QObject(o),
  owner(o),
  networkClient(nullptr)
{
}

int ClientListModelPrivate::findIndexOfClient(const ClientEntity &c) const
{
  auto d = this;
  for (auto i = 0; i < d->clients.size(); ++i)
  {
    if (d->clients[i].id == c.id)
      return i;
  }
  return -1;
}

void ClientListModelPrivate::onClientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  auto d = this;
  d->owner->addClient(client);
}

void ClientListModelPrivate::onClientLeftChannel(const ClientEntity &client, const ChannelEntity &channel)
{
  auto d = this;
  d->owner->removeClient(client);
}

void ClientListModelPrivate::onClientDisconnected(const ClientEntity &client)
{
  auto d = this;
  d->owner->removeClient(client);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

SortFilterClientListProxyModel::SortFilterClientListProxyModel(QObject *parent) :
  QSortFilterProxyModel(parent)
{
}

bool SortFilterClientListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  const auto videoEnabledLeft = left.data(ClientListModel::VideoEnabledRole).toBool();
  const auto videoEnabledRight = right.data(ClientListModel::VideoEnabledRole).toBool();

  if (videoEnabledLeft == videoEnabledRight)
    return left.data().toString().compare(right.data().toString(), Qt::CaseInsensitive) < 0;
  else if (videoEnabledLeft)
    return true;
  else if (videoEnabledRight)
    return false;
  return true;
}
