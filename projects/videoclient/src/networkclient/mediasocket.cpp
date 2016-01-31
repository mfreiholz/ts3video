#include "mediasocket_p.h"

#include <QTimer>
#include <QTimerEvent>
#include "humblelogging/api.h"
#include "medlib/protocol.h"
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
		// Encoding (Delayed start, when the user enables his video)
		connect(d->videoEncodingThread, &VideoEncodingThread::encoded, this, &MediaSocket::onVideoFrameEncoded);

		// Decoding
		d->videoDecodingThread->start();
		connect(d->videoDecodingThread, &VideoDecodingThread::decoded, this, &MediaSocket::onVideoFrameDecoded);
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Audio
	if (true)
	{
		// Encoding
		static quint64 __nextAudioFrameId = 1;
		d->audioEncodingThread->start();
		connect(d->audioEncodingThread, &AudioEncodingThread::encoded, [this](const QByteArray & f, ocs::clientid_t senderId)
		{
			sendAudioFrame(f, __nextAudioFrameId++, senderId);
		});

		// Decoding
		d->audioDecodingThread->start();
		connect(d->audioDecodingThread, &AudioDecodingThread::decoded, this, &MediaSocket::newAudioFrame);
	}
#endif

	// Network usage calculation.
	_bandwidthTimer.setInterval(1500);
	_bandwidthTimer.start();
	QObject::connect(&_bandwidthTimer, &QTimer::timeout, [this]()
	{
		d->networkUsageHelper.recalculate();
		emit networkUsageUpdated(d->networkUsage);
	});
}

MediaSocket::~MediaSocket()
{
	if (_bandwidthTimer.isActive())
		_bandwidthTimer.stop();

	if (d->authenticationTimerId != -1)
		killTimer(d->authenticationTimerId);

	if (d->keepAliveTimerId != -1)
		killTimer(d->keepAliveTimerId);

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

#if defined(OCS_INCLUDE_AUDIO)
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
#endif
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

void MediaSocket::sendVideoFrame(const QImage& image, ocs::clientid_t senderId)
{
	if (!d->videoEncodingThread || !d->videoEncodingThread->isRunning())
	{
		HL_WARN(HL, QString("Can not send video. Encoding thread not yet running.").toStdString());
		return;
	}
	d->videoEncodingThread->enqueue(image, senderId);
}

void MediaSocket::initVideoEncoder(int width, int height, int bitrate, int fps)
{
	if (!d->videoEncodingThread)
		return;
	d->videoEncodingThread->stop();
	d->videoEncodingThread->wait();
	d->videoEncodingThread->init(width, height, bitrate, fps);
	d->videoEncodingThread->start();
}

void MediaSocket::resetVideoEncoder()
{
	if (d->videoEncodingThread)
	{
		d->videoEncodingThread->stop();
		d->videoEncodingThread->wait();
	}
}

void MediaSocket::resetVideoDecoderOfClient(ocs::clientid_t senderId)
{
	if (!d->videoDecodingThread)
		return;
	d->videoDecodingThread->enqueue(nullptr, senderId);
	delete d->videoFrameDatagramDecoders.take(senderId);
}

#if defined(OCS_INCLUDE_AUDIO)
void MediaSocket::sendAudioFrame(const PcmFrameRefPtr& f, ocs::clientid_t senderId)
{
	if (!d->audioEncodingThread || !d->audioEncodingThread->isRunning())
	{
		HL_WARN(HL, QString("Can not send audio. Encoding thread not yet running.").toStdString());
		return;
	}
	d->audioEncodingThread->enqueue(f, senderId);
}
#endif

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

void MediaSocket::sendVideoFrame(const QByteArray& frame_, quint64 frameId_, ocs::clientid_t senderId_)
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

void MediaSocket::sendVideoFrameRecoveryDatagram(quint64 frameId_, ocs::clientid_t fromSenderId_)
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

#if defined(OCS_INCLUDE_AUDIO)
void MediaSocket::sendAudioFrame(const QByteArray& f, quint64 fid, ocs::clientid_t sid)
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
#endif

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
		UDP::Datagram datagram;
		in >> datagram.magic;
		if (datagram.magic != UDP::Datagram::MAGIC)
		{
			HL_WARN(HL, QString("Received invalid datagram (size=%1; data=%2)").arg(data.size()).arg(QString(data)).toStdString());
			continue;
		}

		// Handle by type.
		in >> datagram.type;
		switch (datagram.type)
		{
		//
		// VIDEO
		//
		case UDP::VideoFrameDatagram::TYPE:
		{
			auto dg = new UDP::VideoFrameDatagram();
			in >> dg->flags;
			in >> dg->sender;
			in >> dg->frameId;
			in >> dg->index;
			in >> dg->count;
			in >> dg->size;
			if (dg->size > 0)
			{
				dg->data = new UDP::dg_byte_t[dg->size];
				in.readRawData((char*)dg->data, dg->size);
			}
			else if (dg->size == 0)
			{
				delete dg;
				continue;
			}

			auto senderId = dg->sender;
			auto frameId = dg->frameId;

			// UDP Decode.
			auto decoder = d->videoFrameDatagramDecoders.value(dg->sender);
			if (!decoder)
			{
				decoder = new VideoFrameUdpDecoder();
				d->videoFrameDatagramDecoders.insert(dg->sender, decoder);
			}
			decoder->add(dg);

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
			UDP::VideoFrameRecoveryDatagram dg;
			in >> dg.sender;
			in >> dg.frameId;
			in >> dg.index;
			d->videoEncodingThread->enqueueRecovery();
			break;
		}
#if defined(OCS_INCLUDE_AUDIO)
		//
		// AUDIO
		//
		case UDP::AudioFrameDatagram::TYPE:
		{
			// Parse datagram.
			auto dg = new UDP::AudioFrameDatagram();
			in >> dg->sender;
			in >> dg->frameId;
			in >> dg->index;
			in >> dg->count;
			in >> dg->size;
			if (dg->size > 0)
			{
				dg->data = new UDP::dg_byte_t[dg->size];
				in.readRawData((char*)dg->data, dg->size);
			}
			if (dg->size == 0)
			{
				delete dg;
				continue;
			}

			auto senderId = dg->sender;
			auto frameId = dg->frameId;

			// UDP Decode.
			auto decoder = d->audioFrameDatagramDecoders.value(dg->sender);
			if (!decoder)
			{
				decoder = new AudioUdpDecoder();
				d->audioFrameDatagramDecoders.insert(dg->sender, decoder);
			}
			decoder->add(dg);

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
#endif

		} // switch (type)
	}
}

void MediaSocket::onVideoFrameEncoded(QByteArray frame, ocs::clientid_t senderId)
{
	static quint64 __nextVideoFrameId = 1;
	sendVideoFrame(frame, __nextVideoFrameId++, senderId);
}

void MediaSocket::onVideoFrameDecoded(YuvFrameRefPtr frame, ocs::clientid_t senderId)
{
	emit newVideoFrame(frame, senderId);
}