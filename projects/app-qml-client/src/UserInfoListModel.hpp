#pragma once
#include "libclient/networkclient/clientlistmodel.h"
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <memory>
#include <vector>

class UserInfoListModel : public QIdentityProxyModel
{
	Q_OBJECT
	Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
	enum Roles
	{
		IdRole = Qt::UserRole + 1,
		NameRole,
		VideoEnabledRole,
		VideoWidthRole,
		VideoHeightRole,
		VideoBitrateRole,
	};

	explicit UserInfoListModel(QObject* parent = nullptr);
	~UserInfoListModel() override;
	UserInfoListModel(const UserInfoListModel&) = delete;
	UserInfoListModel& operator=(const UserInfoListModel&) = delete;

	void setClientListModel(ClientListModel* m);
	ClientListModel* clientListModel() const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

signals:
	void countChanged();
	void clientAdded(int clientId);
	void clientRemoved(int clientId);

private:
	QPointer<ClientListModel> m_clientListModel;

	Q_SLOT void onRowsInserted(const QModelIndex& index, int first, int last);
	Q_SLOT void onRowsRemoved(const QModelIndex& index, int first, int last);
};
