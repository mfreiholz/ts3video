#ifndef MEDIASOCKETHANDLER_H
#define MEDIASOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include <QTime>
#include <QString>
#include <QCache>
#include <QBuffer>
#include <QDataStream>
#include <QHostAddress>
#include <QByteArray>
#include <QHash>
#include "baselib/defines.h"
#include "medlib/protocol.h"
#include "videolib/src/networkusageentity.h"

class MediaSenderEntity;
class MediaReceiverEntity;
class MediaRecipients;


class MediaSenderEntity
{
public:
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
	// Maps a SENDER's address+port to itself.
	QHash<QHostAddress, QHash<quint16, MediaSenderEntity> > addr2sender;

	// Maps a RECEIVER's client-id to itself.
	QHash<ocs::clientid_t, MediaReceiverEntity> clientid2receiver;
};


class MediaSocketHandler : public QObject
{
	Q_OBJECT

public:
	MediaSocketHandler(const QHostAddress& address, quint16 port, QObject* parent);
	virtual ~MediaSocketHandler();

	bool init();
	void setRecipients(MediaRecipients&& rec);

signals:
	/*! Emits for every incoming authentication from a client. */
	void tokenAuthentication(const QString& token, const QHostAddress& address, quint16 port);

	/*! Emits whenever the transfer rates have been recalculated and updated.
	    It gets calculated every X seconds by an internal timer. */
	void networkUsageUpdated(const NetworkUsageEntity&);

private slots:
	void onReadyRead();
	void onError(QAbstractSocket::SocketError socketError);

private:
	QHostAddress _address;
	quint16 _port;
	QUdpSocket _socket;
	MediaRecipients _recipients;

	/* onReadyRead() related variables */

	// Socket buffer and cached items.
	// Instead of creating local stacked members we reuse this variables
	// inside onReadyRead() to save allocations.

	char _buffer[4096];
	int _bufferLen = 0;
	
	QByteArray _data;
	QBuffer _dataBuffer;
	QDataStream _in;

	QHostAddress _senderAddress;
	quint16 _senderPort = 0;

	UDP::Datagram _baseDatagram;

	/* network usage */

	NetworkUsageEntity _networkUsage;
	NetworkUsageEntityHelper _networkUsageHelper;
};

#endif
