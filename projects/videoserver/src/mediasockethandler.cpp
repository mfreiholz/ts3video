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

	_in.setByteOrder(QDataStream::BigEndian);

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

	/*  printf("\n");
	    foreach (const auto& senderAddress, rec.addr2sender.keys())
	    {
		foreach (const auto& senderPort, rec.addr2sender[senderAddress].keys())
		{
			printf("FROM %s:%d\n", senderAddress.toString().toStdString().c_str(), senderPort);
			foreach (const auto& receiver, rec.addr2sender[senderAddress][senderPort].receivers)
			{
				printf("\tTO %s:%d\n", receiver.address.toString().toStdString().c_str(), receiver.port);
			}
		}
	    }
	    printf("\n");*/
}

void MediaSocketHandler::onReadyRead()
{
	while (_socket.hasPendingDatagrams())
	{
		_bufferLen = _socket.readDatagram(_buffer, 4096, &_senderAddress, &_senderPort);

		_data.setRawData(_buffer, _bufferLen);

		_dataBuffer.close();
		_dataBuffer.setBuffer(&_data);
		_dataBuffer.open(QIODevice::ReadOnly);

		_in.setDevice(&_dataBuffer);
		_in.resetStatus();

		_networkUsage.bytesRead += _bufferLen;

		_in >> _baseDatagram.magic;
		if (_baseDatagram.magic != UDP::Datagram::MAGIC)
		{
			HL_WARN(HL, QString("Received invalid datagram (size=%1; data=%2)").arg(_data.size()).arg(QString(_data)).toStdString());
			continue;
		}

		_in >> _baseDatagram.type;
		switch (_baseDatagram.type)
		{
			case UDP::AuthDatagram::TYPE:
			{
				UDP::AuthDatagram dgauth;
				_in >> dgauth.size;
				dgauth.data = new UDP::dg_byte_t[dgauth.size];
				auto read = _in.readRawData((char*)dgauth.data, dgauth.size);
				if (read != dgauth.size)
				{
					continue;
				}
				auto token = QString::fromUtf8((char*)dgauth.data, dgauth.size);
				emit tokenAuthentication(token, _senderAddress, _senderPort);
				break;
			}

			case UDP::VideoFrameDatagram::TYPE:
			{
				//const auto senderId = MediaSenderEntity::createIdent(_senderAddress, _senderPort);
				//const auto& senderEntity = _recipients.ident2sender[senderId];
				const auto& senderEntity = _recipients.addr2sender[_senderAddress][_senderPort];
				for (auto i = 0, end = senderEntity.receivers.size(); i < end; ++i)
				{
					const auto& receiverEntity = senderEntity.receivers[i];
					_socket.writeDatagram(_buffer, _bufferLen, receiverEntity.address, receiverEntity.port);
					_networkUsage.bytesWritten += _bufferLen;
				}
				break;
			}

			case UDP::VideoFrameRecoveryDatagram::TYPE:
			{
				UDP::VideoFrameRecoveryDatagram dgrec;
				_in >> dgrec.sender;

				// Send to specific receiver only.
				const auto& receiver = _recipients.clientid2receiver[dgrec.sender];
				if (receiver.address.isNull() || receiver.port == 0)
				{
					HL_WARN(HL, QString("Unknown receiver for recovery frame (client-id=%1)").arg(dgrec.sender).toStdString());
					continue;
				}
				_socket.writeDatagram(_data, receiver.address, receiver.port);
				_networkUsage.bytesWritten += _data.size();
				break;
			}

				/*  case UDP::AudioFrameDatagram::TYPE:
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
				    }*/

		}
	}
}

void MediaSocketHandler::onError(QAbstractSocket::SocketError socketError)
{
	HL_ERROR(HL, QString("socket error (err=%1; message=%2)").arg(socketError).arg(_socket.errorString()).toStdString());
}
