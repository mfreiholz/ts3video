#include "channellistmodel.h"

#include <exception>

#include "videolib/src/jsonprotocolhelper.h"
#include "networkclient/networkclient.h"

ChannelListModel::ChannelListModel(QObject* parent) :
	QAbstractTableModel(parent)
{
	_headers.insert(IdColumn, tr("ID"));
	_headers.insert(NameColumn, tr("Name"));
	_headers.insert(HasPasswordColumn, tr("Password"));
}

ChannelListModel::~ChannelListModel()
{
}

void ChannelListModel::init(const QSharedPointer<NetworkClient>& networkClient)
{
	beginResetModel();
	if (_networkClient)
	{
		_networkClient->disconnect(this);
	}
	_networkClient = networkClient;
	endResetModel();
}

int ChannelListModel::rowCount(const QModelIndex& parent) const
{
	return _channels.count();
}

int ChannelListModel::columnCount(const QModelIndex& parent) const
{
	return _headers.count();
}

QVariant ChannelListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() >= _channels.count())
		return QVariant();

	const auto& channel = _channels.at(index.row());
	switch (index.column())
	{
		case IdColumn:
		{
			switch (role)
			{
				case Qt::DisplayRole:
					return channel.id;
			}
			break;
		}
		case NameColumn:
		{
			switch (role)
			{
			case Qt::DisplayRole:
				return channel.name;
			}
			break;
		}
		case HasPasswordColumn:
		{
			switch (role)
			{
			case Qt::DisplayRole:
				return channel.isPasswordProtected;
			}
			break;
		}
	}
	return QVariant();
}

QVariant ChannelListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}

const ChannelEntity& ChannelListModel::getChannelByIndex(const QModelIndex& index) const
{
	if (index.row() >= _channels.count())
	{
		throw std::exception();
	}
	return _channels.at(index.row());
}

void ChannelListModel::refresh()
{
	if (_networkClient && !_isRefreshing)
	{
		auto reply = _networkClient->getChannelList();
		QObject::connect(reply, &QCorReply::finished, this, &ChannelListModel::onGetChannelListFinished);
		_isRefreshing = true;
	}
}

void ChannelListModel::onGetChannelListFinished()
{
	auto reply = qobject_cast<QCorReply*>(sender());
	reply->deleteLater();
	_isRefreshing = false;

	int status = 0;
	QString error;
	QJsonObject params;
	if (!JsonProtocolHelper::fromJsonResponse(reply->frame()->data(), status, params, error))
		return;
	else if (status != 0)
		return;

	QList<ChannelEntity> channels;
	const QJsonArray jchannels = params["channels"].toArray();
	for (auto i = 0; i < jchannels.count(); ++i)
	{
		ChannelEntity c;
		c.fromQJsonObject(jchannels.at(i).toObject());
		channels.append(std::move(c));
	}

	beginResetModel();
	_channels = channels;
	endResetModel();
}
