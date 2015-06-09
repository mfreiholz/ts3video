#ifndef CLIENTLISTMODEL_H
#define CLIENTLISTMODEL_H

#include <QScopedPointer>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class NetworkClient;
class ClientEntity;
class ChannelEntity;

class ClientListModelPrivate;
class ClientListModel : public QAbstractListModel
{
  Q_OBJECT
  friend class ClientListModelPrivate;
  QScopedPointer<ClientListModelPrivate> d;

public:
  enum Role
  {
    VideoEnabledRole = Qt::UserRole + 1,
    ClientEntityRole = Qt::UserRole + 2
  };

  ClientListModel(QObject *parent);
  virtual ~ClientListModel();

  /*
    Sets the network client to be used for this model.
    Calling this function resets the model and make it base on the new one, but does not delete the old one.
  */
  void setNetworkClient(NetworkClient *networkClient);

  // Helper function to add/edit/remove clients.
  void addClient(const ClientEntity &client);
  void removeClient(const ClientEntity &client);

  // Qt's model pattern overwrites.
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

/*! Provides the list in a sorted way.
*/
class SortFilterClientListProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  SortFilterClientListProxyModel(QObject *parent = nullptr);

protected:
  virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif
