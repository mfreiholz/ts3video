#include "videodecodingthread.h"

#include "humblelogging/api.h"

#include "videolib/ts3video.h"

#include "vp8decoder.h"

HUMBLE_LOGGER(HL, "networkclient.videodecodingthread");

VideoDecodingThread::VideoDecodingThread(QObject* parent) :
	QThread(parent),
	_stopFlag(0)
{
}

VideoDecodingThread::~VideoDecodingThread()
{
	stop();
	wait();

	while (!_queue.isEmpty())
	{
		auto item = _queue.dequeue();
		delete item.first;
	}
}

void VideoDecodingThread::stop()
{
	_stopFlag = 1;
	_queueCond.wakeAll();
}

// Note: Enqueuing an NULL frame, will reset the internal used decoder.
void VideoDecodingThread::enqueue(VP8Frame* frame, ocs::clientid_t senderId)
{
	QMutexLocker l(&_m);
	_queue.enqueue(qMakePair(frame, senderId));
	_queueCond.wakeAll();
}

void VideoDecodingThread::run()
{
	QHash<ocs::clientid_t, VP8Decoder*> decoders;

	_stopFlag = 0;
	while (_stopFlag == 0)
	{
		// Get next frame from queue
		QMutexLocker l(&_m);
		if (_queue.isEmpty())
		{
			_queueCond.wait(&_m);
			continue;
		}
		auto item = _queue.dequeue();
		l.unlock();

		if (/*!item.first || */item.second == 0)
			continue;

		// Delete VP8Decoder, if frame is empty.
		if (!item.first)
		{
			auto decoder = decoders.take(item.second);
			if (decoder)
				delete decoder;
			continue;
		}

		// Get/create decoder
		auto create = false;
		auto decoder = decoders.value(item.second);
		if (!decoder)
			create = true;
		else if (!item.first)
			create = true;

		// Re-/create decoder
		if (create)
		{
			if (decoder)
			{
				decoders.remove(item.second);
				delete decoder;
				decoder = nullptr;
			}

			decoder = new VP8Decoder();
			decoder->initialize();
			decoders.insert(item.second, decoder);
		}
		if (!item.first)
			continue;

		// Decode
		YuvFrameRefPtr yuv(decoder->decodeFrameRaw(item.first->data));
		emit decoded(yuv, item.second);
	}

	// Clean up.
	qDeleteAll(decoders);
	decoders.clear();
}