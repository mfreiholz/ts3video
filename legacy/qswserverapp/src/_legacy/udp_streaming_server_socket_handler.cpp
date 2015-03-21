#include "_legacy/udp_streaming_server_socket_handler.h"

#include <stddef.h>

#include "QtCore/QByteArray"
#include "QtCore/QString"
#include "QtCore/QDataStream"
#include "QtCore/QMutex"
#include "QtCore/QWaitCondition"
#include "QtCore/QList"
#include "QtCore/QPair"
#include "QtCore/QFuture"

#include "QtNetwork/QHostAddress"
#include "QtNetwork/QUdpSocket"

#include "humblelogging/api.h"

#include "_legacy/streamingserver.h"

HUMBLE_LOGGER(logger, "udp");

///////////////////////////////////////////////////////////////////////////////
// Globals.
///////////////////////////////////////////////////////////////////////////////

QString CreateUdpDataReceiverMapKey(const QHostAddress &address, quint16 port)
{
	return address.toString() + QString(":") + QString::number(port);
}

///////////////////////////////////////////////////////////////////////////////
// UdpSocketHandler
///////////////////////////////////////////////////////////////////////////////

class UdpSocketHandler::Private
{
public:
	Private()
		: server(nullptr),
		  socket(nullptr),
		  totalBytesRead(0),
		  totalBytesWritten(0)
	{}

	StreamingServer *server;

	// Socket which is used for all read/write operations.
	QUdpSocket *socket;

	// Total number read and written bytes over this socket.
	quint64 totalBytesRead, totalBytesWritten;

	// Map with associations between connected clients.
	UdpDataReceiverMap receiverMap;
};

UdpSocketHandler::UdpSocketHandler(StreamingServer *server, QObject *parent)
	: QObject(parent), d(new Private())
{
	d->server = server;
	qRegisterMetaType<QHostAddress>("QHostAddress");
	qRegisterMetaType<UdpDataReceiverMap>("UdpDataReceiverMap");
}

UdpSocketHandler::~UdpSocketHandler()
{
	if (d->socket) {
		d->socket->close();
		d->socket->disconnect(this);
		delete d;
		d->socket = nullptr;
	}
}

bool UdpSocketHandler::listen(const QHostAddress &address, quint16 port)
{
	d->socket = new QUdpSocket(this);
	connect(d->socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
	connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged(QAbstractSocket::SocketState)));
	return d->socket->bind(address, port);
}

void UdpSocketHandler::close()
{
	if (d->socket != nullptr)
		d->socket->close();
}

void UdpSocketHandler::setReceiverMap(const UdpDataReceiverMap &map)
{
	d->receiverMap = map;
}

void UdpSocketHandler::clearReceivers()
{
	d->receiverMap.clear();
}

void UdpSocketHandler::readPendingDatagrams()
{
	while (d->socket->hasPendingDatagrams()) {
		QHostAddress sender;
		quint16 senderPort;
		QByteArray *ba = new QByteArray();
		ba->resize(d->socket->pendingDatagramSize());
		const qint64 bytesRead = d->socket->readDatagram(ba->data(), ba->size(), &sender, &senderPort);
		d->totalBytesRead += bytesRead;
		if (!processDatagram(ba, sender, senderPort)) {
			delete ba;
		}
	}
}

void UdpSocketHandler::stateChanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::UnconnectedState) {
		emit done();
	}
}

bool UdpSocketHandler::processDatagram(QByteArray *data, const QHostAddress &sender, quint16 sender_port)
{
	QDataStream in(*data);
	in.setVersion(UdpProtocol::QDS_VERSION);
	UdpProtocol::UdpDatagramHeader header;
	in >> header;
	switch (header.type) {
		case UdpProtocol::DG_TYPE_TOKEN: {
			return processTokenDatagram(in, data, sender, sender_port);
		}
		case UdpProtocol::DG_TYPE_VIDEO_FRAME_V1: {
			return processVideoFrameDatagramV1(in, data, sender, sender_port);
		}
		case UdpProtocol::DG_TYPE_VIDEO_FRAME_RECOVERY_V1: {
			return processVideoFrameRecoveryDatagramV1(in, data, sender, sender_port);
		}
		case UdpProtocol::DG_TYPE_AUDIO_FRAME_V1: {
			return processAudioFrameDatagramV1(in, data, sender, sender_port);
		}
		default: {
			fprintf(stderr, "UdpSocket(%d): Invalid datagram format from %s:%d\n",
					d->socket->localPort(), sender.toString().toStdString().c_str(), sender_port);
		}
	}
	return false;
}

bool UdpSocketHandler::processTokenDatagram(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port)
{
	UdpProtocol::TokenUdpDatagram datagram;
	in >> datagram;
	emit tokenAuthorization(datagram.token, sender, sender_port);
	return true;
}

bool UdpSocketHandler::processVideoFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender_address, quint16 sender_port)
{
	// Find detailed sender information (+receiver list).
	const QString sender_identifier = CreateUdpDataReceiverMapKey(sender_address, sender_port);
	const UdpDataSender &sender_data = d->receiverMap.value(sender_identifier);
	if (sender_data.receivers.isEmpty()) {
		delete data;
		return true;
	}

	// Rewrite senders client-id in package.
	// Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.setVersion(UdpProtocol::QDS_VERSION);
	ds << sender_data.client_id;
	data->replace(sizeof(UdpProtocol::UdpDatagramHeader), sizeof(Protocol::client_id_t), ba);

	// Send data to sibling clients.
	std::for_each(sender_data.receivers.begin(), sender_data.receivers.end(), [&] (const UdpDataReceiver &receiver) {
		const qint64 bytesWritten = d->socket->writeDatagram(*data, receiver.address, receiver.port);
		if (bytesWritten != -1)
			d->totalBytesWritten += bytesWritten;
	});
	delete data;
	return true;
}

bool UdpSocketHandler::processVideoFrameRecoveryDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender_address, quint16 sender_port)
{
	const QString sender_identifier = CreateUdpDataReceiverMapKey(sender_address, sender_port);
	const UdpDataSender &sender_data = d->receiverMap.value(sender_identifier);
	if (sender_data.receivers.isEmpty()) {
		delete data;
		return true;
	}

	// Rewrite senders client-id in package.
	// Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.setVersion(UdpProtocol::QDS_VERSION);
	ds << sender_data.client_id;

	data->replace(sizeof(UdpProtocol::UdpDatagramHeader), 
			sizeof(Protocol::client_id_t), ba);

	// Only send to the client, which has to resend the frame.
	Protocol::client_id_t sender_id, receiver_id;
	in >> sender_id >> receiver_id;

	for (int i = 0; i < sender_data.receivers.size(); ++i) {
		const UdpDataReceiver &receiver = sender_data.receivers.at(i);
		if (receiver.client_id != receiver_id)
			continue;
		const qint64 bytesWritten = d->socket->writeDatagram(*data, receiver.address, receiver.port);
		if (bytesWritten != -1)
			d->totalBytesWritten += bytesWritten;
		break;
	}
	delete data;
	return true;
}

bool UdpSocketHandler::processAudioFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &senderAddress, quint16 senderPort)
{
	// Find detailed sender information (+receiver list).
	const QString sender_identifier = CreateUdpDataReceiverMapKey(senderAddress, senderPort);
	const UdpDataSender &senderData = d->receiverMap.value(sender_identifier);
	if (senderData.receivers.isEmpty()) {
		delete data;
		return true;
	}

	// Rewrite senders client-id in package.
	// Note: Its important to use QDataStream, because it internally swaps BigEndian of some data types!
	QByteArray ba;
	QDataStream ds(&ba, QIODevice::WriteOnly);
	ds.setVersion(UdpProtocol::QDS_VERSION);
	ds << senderData.client_id;
	data->replace(sizeof(UdpProtocol::UdpDatagramHeader), sizeof(Protocol::client_id_t), ba);

	// Send data to sibling clients.
	std::for_each(senderData.receivers.begin(), senderData.receivers.end(), [&] (const UdpDataReceiver &receiver) {
		const qint64 bytesWritten = d->socket->writeDatagram(*data, receiver.address, receiver.port);
		if (bytesWritten != -1)
			d->totalBytesWritten += bytesWritten;
	});
	delete data;
	return true;
}
