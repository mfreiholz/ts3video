#include "mediasocket_p.h"

#include <QTimer>
#include <QTimerEvent>
#include "humblelogging/api.h"
#include "medprotocol.h"
#include "timeutil.h"

HUMBLE_LOGGER(HL, "networkclient.mediasocket");

///////////////////////////////////////////////////////////////////////

#ifdef __linux__
QDataStream& operator<<(QDataStream& out, const UDP::VideoFrameDatagram::dg_frame_id_t& val)
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
}
#endif

///////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////

MediaSocket::MediaSocket(const QString& token, QObject* parent) :
	QUdpSocket(parent),
	d(new MediaSocketPrivate(this))
{
	connect(this, &MediaSocket::stateChanged, this, &MediaSocket::onSocketStateChanged);
	connect(this, static_cast<void(MediaSocket::*)(QAbstractSocket::SocketError)>(&MediaSocket::error), this, &MediaSocket::onSocketError);
	connect(this, &MediaSocket::readyRead, this, &MediaSocket::onReadyRead);

	d->token = token;

	// Video
	if (true)
	{
		// Encoding
		static quint64 __nextVideoFrameId = 1;
		d->videoEncodingThread->start();
		connect(d->videoEncodingThread, &VideoEncodingThread::encoded, [this](const QByteArray & frame, int senderId)
		{
			sendVideoFrame(frame, __nextVideoFrameId++, senderId);
		});

		// Decoding
		d->videoDecodingThread->start();
		connect(d->videoDecodingThread, &VideoDecodingThread::decoded, this, &MediaSocket::newVideoFrame);
	}

	// Audio
	if (true)
	{
		// Encoding
		static quint64 __nextAudioFrameId = 1;
		d->audioEncodingThread->start();
		connect(d->audioEncodingThread, &AudioEncodingThread::encoded, [this](const QByteArray & f, int senderId)
		{
			sendAudioFrame(f, __nextAudioFrameId++, senderId);
		});

		// Decoding
		d->audioDecodingThread->start();
		connect(d->audioDecodingThread, &AudioDecodingThread::decoded, this, &MediaSocket::newAudioFrame);
	}

	// Network usage calculation.
	auto bandwidthTimer = new QTimer(this);
	bandwidthTimer->setInterval(1500);
	bandwidthTimer->start();
	QObject::connect(bandwidthTimer, &QTimer::timeout, [this]()
	{
		d->networkUsageHelper.recalculate();
		emit networkUsageUpdated(d->networkUsage);
	});
}

MediaSocket::~MediaSocket()
{
	if (d->authenticationTimerId != -1)
	{
		killTimer(d->authenticationTimerId);
	}

	if (d->keepAliveTimerId != -1)
	{
		killTimer(d->keepAliveTimerId);
	}

	while (!d->videoFrameDatagramDecoders.isEmpty())
	{
		auto obj = d->videoFrameDatagramDecoders.take(d->videoFrameDatagramDecoders.begin().key());
		delete obj;
	}

	if (d->videoEncodingThread)
	{
		d->videoEncodingThread->stop();
		d->videoEncodingThread->wait();
		delete d->videoEncodingThread;
	}

	if (d->videoDecodingThread)
	{
		d->videoDecodingThread->stop();
		d->videoDecodingThread->wait();
		delete d->videoDecodingThread;
	}

	if (d->audioEncodingThread)
	{
		d->audioEncodingThread->stop();
		d->audioEncodingThread->wait();
		delete d->audioEncodingThread;
	}

	if (d->audioDecodingThread)
	{
		d->audioDecodingThread->stop();
		d->audioDecodingThread->wait();
		delete d->audioDecodingThread;
	}
}

bool MediaSocket::isAuthenticated() const
{
	return d->authenticated;
}

void MediaSocket::setAuthenticated(bool yesno)
{
	d->authenticated = yesno;
	if (d->authenticated && d->authenticationTimerId != -1)
	{
		killTimer(d->authenticationTimerId);
		d->authenticationTimerId = -1;
	}
	if (d->authenticated)
	{
		d->keepAliveTimerId = startTimer(1000);
	}
}

void MediaSocket::sendVideoFrame(const QImage& image, int senderId)
{
	if (!d->videoEncodingThread || !d->videoEncodingThread->isRunning())
	{
		HL_WARN(HL, QString("Can not send video. Encoding thread not yet running.").toStdString());
		return;
	}
	d->videoEncodingThread->enqueue(image, senderId);
}

void MediaSocket::sendAudioFrame(const PcmFrameRefPtr& f, int senderId)
{
	if (!d->audioEncodingThread || !d->audioEncodingThread->isRunning())
	{
		HL_WARN(HL, QString("Can not send audio. Encoding thread not yet running.").toStdString());
		return;
	}
	d->audioEncodingThread->enqueue(f, senderId);
}

void MediaSocket::sendKeepAliveDatagram()
{
	HL_TRACE(HL, QString("Send keep alive datagram").toStdString());

	UDP::KeepAliveDatagram dg;

	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setByteOrder(QDataStream::BigEndian);
	out << dg.magic;
	out << dg.type;

	auto written = writeDatagram(data, peerAddress(), peerPort());
	if (written < 0)
		HL_ERROR(HL, QString("Can not write datagram (error=%1; msg=%2)").arg(error()).arg(errorString()).toStdString());
	else
		d->networkUsage.bytesWritten += written;
}

void MediaSocket::sendAuthTokenDatagram(const QString& token)
{
	HL_TRACE(HL, QString("Send media auth token (token=%1; address=%2; port=%3)").arg(token).arg(peerAddress().toString()).arg(peerPort()).toStdString());
	if (token.isEmpty())
	{
		HL_ERROR(HL, QString("Media auth token is empty").toStdString());
		return;
	}

	UDP::AuthDatagram dg;
	dg.size = token.toUtf8().size();
	dg.data = new UDP::dg_byte_t[dg.size];
	memcpy(dg.data, token.toUtf8().data(), dg.size);

	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	out.setByteOrder(QDataStream::BigEndian);
	out << dg.magic;
	out << dg.type;
	out << dg.size;
	out.writeRawData((char*)dg.data, dg.size);

	auto written = writeDatagram(datagram, peerAddress(), peerPort());
	if (written < 0)
		HL_ERROR(HL, QString("Can not write datagram (error=%1; msg=%2)").arg(error()).arg(errorString()).toStdString());
	else
		d->networkUsage.bytesWritten += written;
}

void MediaSocket::sendVideoFrame(const QByteArray& frame_, quint64 frameId_, quint32 senderId_)
{
	HL_TRACE(HL, QString("Send video frame datagram (frame-size=%1; frame-id=%2; sender-id=%3)").arg(frame_.size()).arg(frameId_).arg(senderId_).toStdString());
	if (frame_.isEmpty() || frameId_ == 0)
	{
		HL_ERROR(HL, QString("Missing data to send video frame (frame-size=%1; frame-id=%2; sender-id=%3)").arg(frame_.size()).arg(frameId_).arg(senderId_).toStdString());
		return;
	}

	UDP::VideoFrameDatagram::dg_frame_id_t frameId = frameId_;
	UDP::VideoFrameDatagram::dg_sender_t senderId = senderId_;

	// Split frame into datagrams.
	UDP::VideoFrameDatagram** datagrams = 0;
	UDP::VideoFrameDatagram::dg_data_count_t datagramsLength;
	if (UDP::VideoFrameDatagram::split((UDP::dg_byte_t*)frame_.data(), frame_.size(), frameId, senderId, &datagrams, datagramsLength) != 0)
	{
		HL_ERROR(HL, QString("Can not split frame data into multiple parts").toStdString());
		return;
	}

	// Send datagrams.
	for (auto i = 0; i < datagramsLength; ++i)
	{
		const auto& dgvideo = *datagrams[i];
		QByteArray datagram;
		QDataStream out(&datagram, QIODevice::WriteOnly);
		out.setByteOrder(QDataStream::BigEndian);
		out << dgvideo.magic;
		out << dgvideo.type;
		out << dgvideo.flags;
		out << dgvideo.sender;
		out << dgvideo.frameId;
		out << dgvideo.index;
		out << dgvideo.count;
		out << dgvideo.size;
		out.writeRawData((char*)dgvideo.data, dgvideo.size);

		auto written = writeDatagram(datagram, peerAddress(), peerPort());
		if (written < 0)
			HL_ERROR(HL, QString("Can not write datagram (error=%1; msg=%2)").arg(error()).arg(errorString()).toStdString());
		else
			d->networkUsage.bytesWritten += written;
	}
	UDP::VideoFrameDatagram::freeData(datagrams, datagramsLength);
}

void MediaSocket::sendVideoFrameRecoveryDatagram(quint64 frameId_, quint32 fromSenderId_)
{
	HL_TRACE(HL, QString("Send video frame recovery datagram (frame-id=%1; from-sender-id=%2)").arg(frameId_).arg(fromSenderId_).toStdString());
	if (frameId_ == 0 || fromSenderId_ == 0)
	{
		HL_ERROR(HL, QString("Missing data to send video recovery frame request (frame-id=%1; from-sender-id=%2)")
				 .arg(frameId_).arg(fromSenderId_).toStdString());
		return;
	}

	UDP::VideoFrameRecoveryDatagram dg;
	dg.sender = fromSenderId_;
	dg.frameId = frameId_;
	dg.index = 0;

	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	out.setByteOrder(QDataStream::BigEndian);
	out << dg.magic;
	out << dg.type;
	out << dg.sender;
	out << dg.frameId;
	out << dg.index;

	auto written = writeDatagram(datagram, peerAddress(), peerPort());
	if (written < 0)
		HL_ERROR(HL, QString("Can not write datagram (error=%1; msg=%2)").arg(error()).arg(errorString()).toStdString());
	else
		d->networkUsage.bytesWritten += written;
}

void MediaSocket::sendAudioFrame(const QByteArray& f, quint64 fid, quint32 sid)
{
	HL_TRACE(HL, QString("Send audio frame datagram (frame-size=%1; frame-id=%2; sender-id=%3)").arg(f.size()).arg(fid).arg(sid).toStdString());
	if (f.isEmpty() || fid == 0)
	{
		HL_ERROR(HL, QString("Missing data to send video frame (frame-size=%1; frame-id=%2; sender-id=%3)").arg(f.size()).arg(fid).arg(sid).toStdString());
		return;
	}

	UDP::AudioFrameDatagram::dg_frame_id_t frameId = fid;
	UDP::AudioFrameDatagram::dg_sender_t senderId = sid;

	// Split frame into datagrams.
	UDP::AudioFrameDatagram** datagrams = 0;
	UDP::AudioFrameDatagram::dg_data_count_t datagramsLength;
	if (UDP::AudioFrameDatagram::split((UDP::dg_byte_t*)f.data(), f.size(), frameId, senderId, &datagrams, datagramsLength) != 0)
	{
		HL_ERROR(HL, QString("Can not split frame data into multiple parts").toStdString());
		return;
	}

	// Send datagrams.
	for (auto i = 0; i < datagramsLength; ++i)
	{
		const auto& dgvideo = *datagrams[i];
		QByteArray datagram;
		QDataStream out(&datagram, QIODevice::WriteOnly);
		out.setByteOrder(QDataStream::BigEndian);
		out << dgvideo.magic;
		out << dgvideo.type;
		out << dgvideo.sender;
		out << dgvideo.frameId;
		out << dgvideo.index;
		out << dgvideo.count;
		out << dgvideo.size;
		out.writeRawData((char*)dgvideo.data, dgvideo.size);

		auto written = writeDatagram(datagram, peerAddress(), peerPort());
		if (written < 0)
			HL_ERROR(HL, QString("Can not write datagram (error=%1; msg=%2)").arg(error()).arg(errorString()).toStdString());
		else
			d->networkUsage.bytesWritten += written;
	}
	UDP::AudioFrameDatagram::freeData(datagrams, datagramsLength);
}

void MediaSocket::timerEvent(QTimerEvent* ev)
{
	if (ev->timerId() == d->authenticationTimerId)
	{
		sendAuthTokenDatagram(d->token);
	}
	else if (ev->timerId() == d->keepAliveTimerId)
	{
		sendKeepAliveDatagram();
	}
}

void MediaSocket::onSocketStateChanged(QAbstractSocket::SocketState state)
{
	HL_TRACE(HL, QString("Socket state changed (state=%1)").arg(state).toStdString());
	switch (state)
	{
	case QAbstractSocket::ConnectedState:
		if (d->authenticationTimerId == -1)
		{
			d->authenticationTimerId = startTimer(1000);
		}
		break;
	case QAbstractSocket::UnconnectedState:
		if (d->authenticationTimerId != -1)
		{
			killTimer(d->authenticationTimerId);
			d->authenticationTimerId = -1;
		}
		if (d->keepAliveTimerId != -1)
		{
			killTimer(d->keepAliveTimerId);
			d->keepAliveTimerId = -1;
		}
		break;
	}
}

void MediaSocket::onSocketError(QAbstractSocket::SocketError error)
{
	HL_ERROR(HL, QString("Socket error (error=%1; message=%2)").arg(error).arg(errorString()).toStdString());
}

void MediaSocket::onReadyRead()
{
	while (hasPendingDatagrams())
	{
		// Read datagram.
		QByteArray data;
		QHostAddress senderAddress;
		quint16 senderPort;
		data.resize(pendingDatagramSize());
		auto read = readDatagram(data.data(), data.size(), &senderAddress, &senderPort);
		if (read <= 0)
			continue;
		d->networkUsage.bytesRead += read;

		QDataStream in(data);
		in.setByteOrder(QDataStream::BigEndian);

		// Check magic.
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
		case UDP::VideoFrameDatagram::TYPE:
		{
			// Parse datagram.
			auto dgvideo = new UDP::VideoFrameDatagram();
			in >> dgvideo->flags;
			in >> dgvideo->sender;
			in >> dgvideo->frameId;
			in >> dgvideo->index;
			in >> dgvideo->count;
			in >> dgvideo->size;
			if (dgvideo->size > 0)
			{
				dgvideo->data = new UDP::dg_byte_t[dgvideo->size];
				in.readRawData((char*)dgvideo->data, dgvideo->size);
			}
			if (dgvideo->size == 0)
			{
				delete dgvideo;
				continue;
			}

			auto senderId = dgvideo->sender;
			auto frameId = dgvideo->frameId;

			// UDP Decode.
			auto decoder = d->videoFrameDatagramDecoders.value(dgvideo->sender);
			if (!decoder)
			{
				decoder = new VideoFrameUdpDecoder();
				d->videoFrameDatagramDecoders.insert(dgvideo->sender, decoder);
			}
			decoder->add(dgvideo);

			// Check for new decoded frame.
			auto frame = decoder->next();
			auto waitForType = decoder->getWaitsForType();
			if (frame)
			{
				d->videoDecodingThread->enqueue(frame, senderId);
			}

			// Handle the case, that the UDP decoder requires some special data.
			if (waitForType != VP8Frame::NORMAL)
			{
				// Request recovery frame (for now only key-frames).
				auto now = get_local_timestamp();
				if (get_local_timestamp_diff(d->lastFrameRequestTimestamp, now) > 1000)
				{
					d->lastFrameRequestTimestamp = now;
					sendVideoFrameRecoveryDatagram(frameId, senderId);
				}
			}
			break;
		}

		case UDP::VideoFrameRecoveryDatagram::TYPE:
		{
			UDP::VideoFrameRecoveryDatagram dgrec;
			in >> dgrec.sender;
			in >> dgrec.frameId;
			in >> dgrec.index;
			d->videoEncodingThread->enqueueRecovery();
			break;
		}

		case UDP::AudioFrameDatagram::TYPE:
		{
			// Parse datagram.
			auto dgvideo = new UDP::AudioFrameDatagram();
			in >> dgvideo->sender;
			in >> dgvideo->frameId;
			in >> dgvideo->index;
			in >> dgvideo->count;
			in >> dgvideo->size;
			if (dgvideo->size > 0)
			{
				dgvideo->data = new UDP::dg_byte_t[dgvideo->size];
				in.readRawData((char*)dgvideo->data, dgvideo->size);
			}
			if (dgvideo->size == 0)
			{
				delete dgvideo;
				continue;
			}

			auto senderId = dgvideo->sender;
			auto frameId = dgvideo->frameId;

			// UDP Decode.
			auto decoder = d->audioFrameDatagramDecoders.value(dgvideo->sender);
			if (!decoder)
			{
				decoder = new AudioUdpDecoder();
				d->audioFrameDatagramDecoders.insert(dgvideo->sender, decoder);
			}
			decoder->add(dgvideo);

			// Check for new decoded frame.
			auto frame = decoder->next();
			if (frame)
			{
				OpusFrameRefPtr p(frame);
				d->audioDecodingThread->enqueue(p, senderId);
			}

			//// Handle the case, that the UDP decoder requires some special data.
			//auto waitForType = decoder->getWaitsForType();
			//if (waitForType != VP8Frame::NORMAL)
			//{
			//	// Request recovery frame (for now only key-frames).
			//	auto now = get_local_timestamp();
			//	if (get_local_timestamp_diff(d->lastFrameRequestTimestamp, now) > 1000)
			//	{
			//		d->lastFrameRequestTimestamp = now;
			//		sendVideoFrameRecoveryDatagram(frameId, senderId);
			//	}
			//}
			break;
		}
		} // switch (type)
	}
}