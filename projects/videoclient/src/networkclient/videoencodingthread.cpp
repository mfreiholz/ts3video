#include "videoencodingthread.h"
#include <QTime>
#include "humblelogging/api.h"
#include "ts3video.h"
#include "vp8frame.h"
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

void VideoEncodingThread::enqueue(const QImage& image, int senderId)
{
	QMutexLocker l(&_m);
	_queue.enqueue(qMakePair(image, senderId));
	while (_queue.size() > 5)
		auto item = _queue.dequeue();
	_queueCond.wakeAll();
}

void VideoEncodingThread::enqueueRecovery()
{
	_recoveryFlag = VP8Frame::KEY;
	_queueCond.wakeAll();
}

void VideoEncodingThread::run()
{
	const int fps = 24;
	const int fpsTimeMs = 1000 / fps;
	const int bitRate = 100;

	QHash<int, VP8Encoder*> encoders;
	QTime fpsTimer;
	fpsTimer.start();

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

		if (item.first.isNull())
			continue;


		// FPS check
		if (fps > 0 && fpsTimer.elapsed() < fpsTimeMs)
			continue;
		fpsTimer.restart();


		// Convert to YuvFrame.
		auto yuv = YuvFrame::fromQImage(item.first);

		// Get/create encoder
		auto create = false;
		auto encoder = encoders.value(item.second);
		if (!encoder)
			create = true;
		else if (!encoder->isValidFrame(*yuv))
			create = true;

		// Re-/create encoder
		if (create)
		{
			if (encoder)
			{
				encoders.remove(item.second);
				delete encoder;
				encoder = nullptr;
			}

			HL_DEBUG(HL, QString("Create new video encoder (id=%1)").arg(item.second).toStdString());
			encoder = new VP8Encoder();
			if (!encoder->initialize(yuv->width, yuv->height, bitRate, fps))
			{
				HL_ERROR(HL, QString("Can not initialize VP8 video encoder").toStdString());
				_stopFlag = 1;
				continue;
			}
			encoders.insert(item.second, encoder);
		}

		if (_recoveryFlag != VP8Frame::NORMAL)
		{
			encoder->setRequestRecoveryFlag(VP8Frame::KEY);
			_recoveryFlag = VP8Frame::NORMAL;
		}

		// Encode frame
		auto vp8 = encoder->encode(*yuv);
		delete yuv;

		// Serialize VP8Frame.
		QByteArray data;
		QDataStream out(&data, QIODevice::WriteOnly);
		out << *vp8;
		delete vp8;
		emit encoded(data, item.second);

		// DEV Provides plain QImage.
		//if (true) {
		//  static quint64 __frameTime = 1;
		//  VP8Frame vpframe;
		//  vpframe.time = __frameTime++;
		//  vpframe.type = VP8Frame::KEY;
		//  QDataStream out(&vpframe.data, QIODevice::WriteOnly);
		//  out << item.first;

		//  QByteArray data;
		//  QDataStream out2(&data, QIODevice::WriteOnly);
		//  out2 << vpframe;

		//  emit encoded(data, item.second);
		//}

	}

	// Clean up.
	qDeleteAll(encoders);
	encoders.clear();
}