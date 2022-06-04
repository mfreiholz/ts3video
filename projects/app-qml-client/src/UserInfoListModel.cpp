#include "UserInfoListModel.hpp"
#include "libapp/cliententity.h"
#include <QtCore/QVariant>

UserInfoListModel::UserInfoListModel(QObject* parent)
	: QIdentityProxyModel(parent)
{
	QObject::connect(this, &UserInfoListModel::rowsInserted, this, &UserInfoListModel::onRowsInserted);
	QObject::connect(this, &UserInfoListModel::rowsRemoved, this, &UserInfoListModel::onRowsRemoved);
}

UserInfoListModel::~UserInfoListModel()
{}

void UserInfoListModel::setClientListModel(ClientListModel* m)
{
	m_clientListModel = m;
	setSourceModel(m);
}

ClientListModel* UserInfoListModel::clientListModel() const
{
	return m_clientListModel;
}

QVariant UserInfoListModel::data(const QModelIndex& index, int role) const
{
	const QVariant vt = m_clientListModel->data(index, ClientListModel::ClientEntityRole);
	if (!vt.isValid())
		return vt;

	const ClientEntity entity = vt.value<ClientEntity>();
	switch (role)
	{
		case Roles::IdRole:
			return entity.id;
		case Roles::NameRole:
			return entity.name;
	}
	return QIdentityProxyModel::data(index, role);
}

QHash<int, QByteArray> UserInfoListModel::roleNames() const
{
	QHash<int, QByteArray> h;
	h[Roles::IdRole] = "clientId";
	h[Roles::NameRole] = "name";
	//h[Roles::VideoEnabledRole] = "videoEnabledRole";
	//h[Roles::VideoWidthRole] = "videoWidthRole";
	//h[Roles::VideoHeightRole] = "videoHeightRole";
	//h[Roles::VideoBitrateRole] = "videoBitrateRole";
	return h;
}

void UserInfoListModel::onRowsInserted(const QModelIndex& parent, int first, int last)
{
	Q_ASSERT(first == last);
	const auto vtClientId = data(index(first, 0, parent), Roles::IdRole);
	if (vtClientId.isValid())
		emit clientAdded(vtClientId.toInt());
	emit countChanged();
}

void UserInfoListModel::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
	Q_ASSERT(first == last);
	const auto vtClientId = data(index(first, 0, parent), Roles::IdRole);
	if (vtClientId.isValid())
		emit clientRemoved(vtClientId.toInt());
	emit countChanged();
}
