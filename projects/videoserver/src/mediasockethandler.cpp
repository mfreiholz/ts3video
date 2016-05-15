#include "mediasockethandler.h"

#include <QString>
#include <QDataStream>
#include <QTimer>
#include "humblelogging/api.h"
#include "virtualserver.h"

HUMBLE_LOGGER(HL, "server.mediasocket");

#ifdef __linux__
/*  QDataStream& operator<<(QDataStream& out, const UDP::VideoFrameDatagram::dg_frame_id_t& val)
    {
	out << (quint64)val;
	return out;
    }

    QDataStream& operator>>(QDataStream& in, UDP::VideoFrameDatagram::dg_frame_id_t& val)
    {
	quint64 i;
	in >> i;
	val = i;
	return in;
    }*/
#endif

///////////////////////////////////////////////////////////////////////

MediaSocketHandler::MediaSocketHandler(const QHostAddress& address, quint16 port, QObject* parent) :
	QObject(parent),
	_address(address),
	_port(port),
	_socket(this),
	_networkUsage(),
	_networkUsageHelper(_networkUsage)
{
	connect(&_socket, &QUdpSocket::readyRead, this, &MediaSocketHandler::onReadyRead);
	connect(&_socket, static_cast<void(QUdpSocket::*)(QAbstractSocket::SocketError)>(&QUdpSocket::error), this, &MediaSocketHandler::onError);

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

void MediaSocketHandler::setRecipients(MediaRecipients&& rec)
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

		QDataStream in(data);
		in.setByteOrder(QDataStream::BigEndian);

		UDP::Datagram dg;
		in >> dg.magic;
		if (dg.magic != UDP::Datagram::MAGIC)
		{
			HL_WARN(HL, QString("Received invalid datagram (size=%1; data=%2)").arg(data.size()).arg(QString(data)).toStdString());
			continue;
		}

		in >> dg.type;
		switch (dg.type)
		{
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

			case UDP::VideoFrameDatagram::TYPE:
			{
				const auto senderId = MediaSenderEntity::createIdent(senderAddress, senderPort);
				const auto& senderEntity = _recipients.ident2sender[senderId];
				for (auto i = 0, end = senderEntity.receivers.size(); i < end; ++i)
				{
					const auto& receiverEntity = senderEntity.receivers[i];
					_socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
					_networkUsage.bytesWritten += data.size();
				}
				break;
			}

			case UDP::VideoFrameRecoveryDatagram::TYPE:
			{
				UDP::VideoFrameRecoveryDatagram dgrec;
				in >> dgrec.sender;

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

			case UDP::AudioFrameDatagram::TYPE:
			{
				const auto senderId = MediaSenderEntity::createIdent(senderAddress, senderPort);
				const auto& senderEntity = _recipients.ident2sender[senderId];
				for (auto i = 0; i < senderEntity.receivers.size(); ++i)
				{
					const auto& receiverEntity = senderEntity.receivers[i];
					_socket.writeDatagram(data, receiverEntity.address, receiverEntity.port);
					_networkUsage.bytesWritten += data.size();
				}
				break;
			}

		}

	}
}

void MediaSocketHandler::onError(QAbstractSocket::SocketError socketError)
{
	HL_ERROR(HL, QString("socket error (err=%1; message=%2)").arg(socketError).arg(_socket.errorString()).toStdString());
}
