#if defined(OCS_INCLUDE_AUDIO)
#include "audioencodingthread.h"

#include <QMutexLocker>
#include <QHash>
#include <QString>
#include <memory>
#include "humblelogging/api.h"
#include "opusencoder.h"

HUMBLE_LOGGER(HL, "networkclient.audioencodingthread");

AudioEncodingThread::AudioEncodingThread(QObject* parent) :
	_stopFlag(0), _recoveryFlag(OpusFrame::NORMAL)
{
}

AudioEncodingThread::~AudioEncodingThread()
{
	stop();
	wait();
}

void AudioEncodingThread::stop()
{
	_stopFlag = 1;
	_queueCond.wakeAll();
}

void AudioEncodingThread::enqueue(const PcmFrameRefPtr& f, int senderId)
{
	QMutexLocker l(&_m);
	_queue.append(qMakePair(f, senderId));
	while (_queue.size() > 20)
	{
		_queue.dequeue();
	}
	_queueCond.wakeAll();
}

void AudioEncodingThread::enqueueRecovery()
{
	// TODO Not yet implemented!
}

void AudioEncodingThread::run()
{
	QHash<int, OpusAudioEncoder*> encoders;
	
	_stopFlag = 0;
	while (_stopFlag == 0)
	{
		QMutexLocker l(&_m);
		if (_queue.isEmpty())
		{
			_queueCond.wait(&_m);
			continue;
		}
		auto item = _queue.dequeue();
		l.unlock();

		if (item.first.isNull())
			continue;

		// Encoder.
		auto encoder = encoders.value(item.second);
		if (!encoder)
		{
			HL_DEBUG(HL, QString("Create new Opus audio encoder (id=%1)").arg(item.second).toStdString());
			encoder = new OpusAudioEncoder();
			encoders.insert(item.second, encoder);
		}

		// Recover?
		if (_recoveryFlag != OpusFrame::NORMAL)
		{
			//encoder->setRecovery..
			//_recoveryFlag = OpusFrame::NORMAL;
		}

		// Encode!
		auto f = std::unique_ptr<OpusFrame>(encoder->encode(*item.first.data()));

		// Serialize for network transfer.
		QByteArray data;
		QDataStream out(&data, QIODevice::WriteOnly);
		out << *f.get();
		emit encoded(data, item.second);
	}

	qDeleteAll(encoders);
	encoders.clear();
}

#endif
