#include "videoencodingthread.h"

#include <QTime>

#include "humblelogging/api.h"

#include "videolib/ts3video.h"

#include "vp8encoder.h"

HUMBLE_LOGGER(HL, "networkclient.videoencodingthread");

VideoEncodingThread::VideoEncodingThread(QObject* parent) :
	QThread(parent),
	_stopFlag(0),
	_recoveryFlag(VP8Frame::NORMAL)
{
}

VideoEncodingThread::~VideoEncodingThread()
{
	stop();
	wait();
}

void VideoEncodingThread::init(int width, int height, int bitrate, int fps)
{
	QMutexLocker l(&_m);
	_width = width;
	_height = height;
	_bitrate = bitrate;
	_fps = fps;
}

void VideoEncodingThread::stop()
{
	_stopFlag = 1;
	_queueCond.wakeAll();
}

void VideoEncodingThread::enqueue(const QImage& image, ocs::clientid_t senderId)
{
	QMutexLocker l(&_m);
	_queue.enqueue(qMakePair(image, senderId));
	while (_queue.size() > 5)
		_queue.dequeue();
	_queueCond.wakeAll();
}

void VideoEncodingThread::enqueueRecovery(VP8Frame::FrameType ft)
{
	_recoveryFlag = ft;
	_queueCond.wakeAll();
}

void VideoEncodingThread::run()
{
	QMutexLocker l(&_m);
	const auto width = _width;
	const auto height = _height;
	const auto bitrate = _bitrate;
	const auto fps = _fps;
	const auto fpsTimeMs = 1000 / fps;
	l.unlock();

	QScopedPointer<VP8Encoder> encoder;
	QTime fpsTimer;
	fpsTimer.start();

	_stopFlag = 0;
	while (_stopFlag == 0)
	{
		// Get next frame from queue
		l.relock();
		if (_queue.isEmpty())
		{
			_queueCond.wait(&_m);
			continue;
		}
		auto item = _queue.dequeue();
		l.unlock();

		if (item.first.isNull())
			continue;


		// FPS check
		// TODO Instead of using this timer, we should grab frames from camera with FPS.
		if (fps > 0 && fpsTimer.elapsed() < fpsTimeMs)
			continue;
		fpsTimer.restart();


		// Convert to YuvFrame.
		const QScopedPointer<YuvFrame> yuv(YuvFrame::fromQImage(item.first));

		// Get/create encoder
		auto create = false;
		if (!encoder)
			create = true;
		else if (!encoder->isValidFrame(*yuv))
			create = true;

		// Re-/create encoder
		if (create)
		{
			if (encoder)
				encoder.reset();

			encoder.reset(new VP8Encoder());
			if (!encoder->initialize(width, height, bitrate, fps))
			{
				_stopFlag = 1;
				emit error(QString("Can not initialize video encoder"));
				continue;
			}
		}

		if (_recoveryFlag != VP8Frame::NORMAL)
		{
			encoder->setRequestRecoveryFlag(VP8Frame::KEY);
			_recoveryFlag = VP8Frame::NORMAL;
		}

		// Encode frame
		const QScopedPointer<VP8Frame> vp8(encoder->encode(*yuv));

		// Serialize VP8Frame.
		QByteArray data;
		QDataStream out(&data, QIODevice::WriteOnly);
		out << *vp8;
		emit encoded(data, item.second);
	}
}