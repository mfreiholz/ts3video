#ifndef _UDP_STREAMING_SERVER_SOCKET_HANDLER_HEADER_
#define _UDP_STREAMING_SERVER_SOCKET_HANDLER_HEADER_

#include "QtCore/QObject"
#include "QtCore/QHash"
#include "QtCore/QSharedPointer"
#include "QtCore/QList"
#include "QtCore/QVector"

#include "QtNetwork/QAbstractSocket"
#include "QtNetwork/QHostAddress"

#include "shared/network/udp_streaming_protocol.h"
#include "shared/network/clientinfo.h"

class QString;
class QByteArray;
class QDataStream;
class QHostAddress;
class StreamingServer;
class UdpDataSender;
class UdpDataReceiver;

// QHash which associates a sender to all receivers.
// Key = String concatination of "<ip-address>:<port>".
typedef QHash<QString, UdpDataSender> UdpDataReceiverMap;
QString CreateUdpDataReceiverMapKey(const QHostAddress &address, quint16 port);

class UdpSocketHandler : public QObject
{
	Q_OBJECT
public:
	UdpSocketHandler(StreamingServer *server, QObject *parent = 0);
	~UdpSocketHandler();

	bool listen(const QHostAddress &address, quint16 port);
	void close();

public slots:
	void setReceiverMap(const UdpDataReceiverMap &map);
	void clearReceivers();

private slots:
	void readPendingDatagrams();
	void stateChanged(QAbstractSocket::SocketState state);

private:
	bool processDatagram(QByteArray *data, const QHostAddress &sender, quint16 sender_port);
	bool processTokenDatagram(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);
	bool processVideoFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);
	bool processVideoFrameRecoveryDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &senderAddress, quint16 senderPort);
	bool processAudioFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);

signals:
	void tokenAuthorization(const QString &token, const QHostAddress &sender, quint16 senderPort);
	void readyWrite();
	void done();

private:
	class Private;
	Private *d;
};


class UdpDataSender
{
public:
	Protocol::client_id_t client_id;
	QVector<UdpDataReceiver> receivers;
};


class UdpDataReceiver
{
public:
	Protocol::client_id_t client_id;
	QString identifier;
	QHostAddress address;
	quint16 port;
};


Q_DECLARE_METATYPE(UdpDataSender)
Q_DECLARE_METATYPE(UdpDataReceiver)
Q_DECLARE_METATYPE(UdpDataReceiverMap)
#endif
