#ifndef MEDIASOCKETHANDLER_H
#define MEDIASOCKETHANDLER_H

#include <memory>

#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include <QTime>
#include <QString>
#include <QCache>

#include "baselib/defines.h"

#include "medlib/protocol.h"

#include "networkusageentity.h"

class MediaSenderEntity;
class MediaReceiverEntity;
class MediaRecipients;


class MediaSenderEntity
{
public:
	static inline QString createIdent(const QHostAddress& address, quint16 port)
	{
		return address.toString() + QString(":") + QString::number(port);
	}

	QString ident;
	ocs::clientid_t clientId;
	QHostAddress address;
	quint16 port;
	QVector<MediaReceiverEntity> receivers;
};


class MediaReceiverEntity
{
public:
	ocs::clientid_t clientId;
	QHostAddress address;
	quint16 port;
};


class MediaRecipients
{
public:
	QHash<QString, MediaSenderEntity> ident2sender; ///< Maps a sender's adress-id to itself.
	QHash<ocs::clientid_t, MediaReceiverEntity> clientid2receiver; ///< Maps a receiver's client-id to itself.
};


class VideoCacheItem
{
public:
	static inline QString createKeyFor(const UDP::VideoFrameDatagram& vfd)
	{
		QString s;
		s.append(QString::number(vfd.sender));
		s.append(":");
		s.append(QString::number(vfd.frameId));
		s.append(":");
		s.append(QString::number(vfd.index));
		return s;
	}

public:
	QByteArray data; ///< Plain data of datagram
	std::unique_ptr<UDP::VideoFrameDatagram> datagram; ///< Parsed information from "data"
};

/*! Receives video-streams and forwards them to all clients in the same channel.
*/
class MediaSocketHandler : public QObject
{
	Q_OBJECT

public:
	MediaSocketHandler(const QHostAddress& address, quint16 port, QObject* parent);
	~MediaSocketHandler();
	bool init();
	void setRecipients(const MediaRecipients& rec);

signals:
	/*! Emits for every incoming authentication from a client.
	*/
	void tokenAuthentication(const QString& token, const QHostAddress& address, quint16 port);

	/*! Emits whenever the transfer rates have been recalculated and updated.
	    It gets calculated every X seconds by an internal timer.
	*/
	void networkUsageUpdated(const NetworkUsageEntity&);

private slots:
	void onReadyRead();

private:
	QHostAddress _address;
	quint16 _port;
	QUdpSocket _socket;
	MediaRecipients _recipients;

	// Caches
	QCache<QString, VideoCacheItem> _videoCache;

	// Network usage.
	NetworkUsageEntity _networkUsage;
	NetworkUsageEntityHelper _networkUsageHelper;
};

#endif