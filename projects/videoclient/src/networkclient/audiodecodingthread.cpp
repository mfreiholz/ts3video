#if defined(OCS_INCLUDE_AUDIO)
#include "audiodecodingthread.h"

#include <QMutexLocker>
#include <QHash>
#include <QString>
#include "humblelogging/api.h"
#include "opusdecoder.h"

HUMBLE_LOGGER(HL, "networkclient.audiodecodingthread");

AudioDecodingThread::AudioDecodingThread(QObject* parent) :
	QThread(parent), _stopFlag(0)
{
}

AudioDecodingThread::~AudioDecodingThread()
{
	stop();
	wait();
}

void AudioDecodingThread::stop()
{
	_stopFlag = 1;
	_queueCond.wakeAll();
}

void AudioDecodingThread::enqueue(const OpusFrameRefPtr& f, int senderId)
{
	QMutexLocker l(&_m);
	_queue.enqueue(qMakePair(f, senderId));
	_queueCond.wakeAll();
}

void AudioDecodingThread::run()
{
	QHash<int, OpusAudioDecoder*> decoders;

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

		if (item.first.isNull() || item.second == 0)
			continue;

		// Decoder
		auto dec = decoders.value(item.second);
		if (!dec)
		{
			HL_DEBUG(HL, QString("Create new Opus audio decoder (id=%1)").arg(item.second).toStdString());
			dec = new OpusAudioDecoder();
			decoders.insert(item.second, dec);
		}

		// Decode
		PcmFrameRefPtr pcm(dec->decode(*item.first.data()));
		emit decoded(pcm, item.second);
	}

	qDeleteAll(decoders);
	decoders.clear();
}

#endif
