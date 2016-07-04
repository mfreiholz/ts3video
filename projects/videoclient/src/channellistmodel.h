#ifndef CHANNELLISTMODEL_H
#define CHANNELLISTMODEL_H

#include <QSharedPointer>
#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QString>

#include "videolib/channelentity.h"

class NetworkClient;

class ChannelListModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum Column { IdColumn, NameColumn, HasPasswordColumn };

	ChannelListModel(QObject* parent = nullptr);
	virtual ~ChannelListModel();
	void init(const QSharedPointer<NetworkClient>& networkClient);

	// Qt's model overwrites.
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	// Helper methods.
	/*!
		\throws std::exception if there is no channel.
	*/
	const ChannelEntity& getChannelByIndex(const QModelIndex& index) const;

public slots:
	void refresh();

private slots:
	void onGetChannelListFinished();

private:
	QSharedPointer<NetworkClient> _networkClient;
	QList<ChannelEntity> _channels;
	QMap<int, QString> _headers;

	bool _isRefreshing = false;
};

#endif
