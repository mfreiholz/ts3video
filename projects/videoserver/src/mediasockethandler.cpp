#include "mediasockethandler.h"

#include <QString>
#include <QDataStream>
#include <QTimer>

#include "humblelogging/api.h"

#include "virtualserver.h"

HUMBLE_LOGGER(HL, "server.mediasocket");

///////////////////////////////////////////////////////////////////////

MediaSocketHandler::MediaSocketHandler(const QHostAddress& address, quint16 port, QObject* parent) :
	QObject(parent),
	_address(address),
	_port(port),
	_socket(this),
	_videoCache(10485760), // 10 MB
	_networkUsage(),
	_networkUsageHelper(_networkUsage)
{
	connect(&_socket, &QUdpSocket::readyRead, this, &MediaSocketHandler::onReadyRead);

	// Update bandwidth status every X seconds.
	auto bandwidthTimer = new QTimer(this);
	bandwidthTimer->setInterval(1500);
	bandwidthTimer->start();
	QObject::connect(bandwidthTimer, &QTimer::timeout, [this]()
	{
		_networkUsageHelper.recalculate();
		emit networkUsageUpdated(_networkUsage);
	});
}

MediaSocketHandler::~MediaSocketHandler()
{
	_socket.close();
}

bool MediaSocketHandler::init()
{
	if (!_socket.bind(_address, _port, QAbstractSocket::DontShareAddress))
	{
		HL_ERROR(HL, QString("Can not bind to UDP port (port=%1)").arg(_port).toStdString());
		return false;
	}
	return true;
}

void MediaSocketHandler::setRecipients(const MediaRecipients& rec)
{
	_recipients = rec;

	//printf("\n");
	//foreach (const auto& sender, rec.id2sender.values())
	//{
	//	printf("FROM %s\n", sender.id.toStdString().c_str());
	//	foreach (const auto& receiver, sender.receivers)
	//	{
	//		printf("\tTO %s:%d\n", receiver.address.toString().toStdString().c_str(), receiver.port);
	//	}
	//}
	//printf("\n");
}

void MediaSocketHandler::onReadyRead()
{
	while (_socket.hasPendingDatagrams())
	{
		// Read datagram.
		QByteArray data;
		QHostAddress senderAddress;
		quint16 senderPort;
		data.resize(_socket.pendingDatagramSize());
		_socket.readDatagram(data.data(), data.size(), &senderAddress, &senderPort);
		_networkUsage.bytesRead += data.size();

		// Check magic.
		QDataStream in(data);
		in.setByteOrder(QDataStream::BigEndian);

		UDP::Datagram dg;
		in >> dg.magic;
		if (dg.magic != UDP::Datagram::MAGIC)
		{
			HL_WARN(HL, QString("Received invalid datagram (size=%1; data=%2)").arg(data.size()).arg(QString(data)).toStdString());
			continue;
		}

		// Handle by type.
		in >> dg.type;
		switch (dg.type)
		{
		// Authentication
		case UDP::AuthDatagram::TYPE:
		{
			UDP::AuthDatagram dgauth;
			in >> dgauth.size;
			dgauth.data = new UDP::dg_byte_t[dgauth.size];
			auto read = in.readRawData((char*)dgauth.data, dgauth.size);
			if (read != dgauth.size)
			{
				// Error.
				continue;
			}
			auto token = QString::fromUtf8((char*)dgauth.data, dgauth.size);
			emit tokenAuthentication(token, senderAddress, senderPort);
			break;
		}

		// Video data.
		case UDP::VideoFrameDatagram::TYPE:
		{
			if (_videoCache.maxCost() > 0)
			{
				// Parse datagram
				auto vfd = std::unique_ptr<UDP::VideoFrameDatagram>(new UDP::VideoFrameDatagram());
				in >> vfd->flags;
				in >> vfd->sender;
				in >> vfd->frameId;
				in >> vfd->index;
				in >> vfd->count;
				in >> vfd->size;

				// Cache frame
				// No longer access "vfd" after move to cacheItem!
				auto cacheItem = new VideoCacheItem();
				cacheItem->data = data;
				cacheItem->datagram = std::move(vfd);
				_videoCache.insert(VideoCacheItem::createKeyFor(*cacheItem->datagram.get()), cacheItem, data.size() + sizeof(*cacheItem->datagram.get()));
			}

			// Broadcast
			const auto senderId = MediaSenderEntity::createID(senderAddress, senderPort);
			const auto& senderEntity = _recipients.id2sender[senderId];
			for (auto i = 0, end = senderEntity.receivers.size(); i < end; ++i)
			{
				const auto& receiverEntity = senderEntity.receivers[i];
				_socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
				_networkUsage.bytesWritten += data.size();
			}
			break;
		}

		// Video recovery.
		case UDP::VideoFrameRecoveryDatagram::TYPE:
		{
			HL_TRACE(HL, QString("Process video frame recovery datagram.").toStdString());

			UDP::VideoFrameRecoveryDatagram dgrec;
			in >> dgrec.sender;
			//in >> dgrec.frameId;
			//in >> dgrec.index;

			// Send to specific receiver only.
			const auto& receiver = _recipients.clientid2receiver[dgrec.sender];
			if (receiver.address.isNull() || receiver.port == 0)
			{
				HL_WARN(HL, QString("Unknown receiver for recovery frame (client-id=%1)").arg(dgrec.sender).toStdString());
				continue;
			}
			_socket.writeDatagram(data, receiver.address, receiver.port);
			_networkUsage.bytesWritten += data.size();
			break;
		}

		// Audio data.
		case UDP::AudioFrameDatagram::TYPE:
		{
			const auto senderId = MediaSenderEntity::createID(senderAddress, senderPort);
			const auto& senderEntity = _recipients.id2sender[senderId];
			for (auto i = 0; i < senderEntity.receivers.size(); ++i)
			{
				const auto& receiverEntity = senderEntity.receivers[i];
				_socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
				_networkUsage.bytesWritten += data.size();
			}
			break;
		}

		}

	} // while (datagrams)
}
