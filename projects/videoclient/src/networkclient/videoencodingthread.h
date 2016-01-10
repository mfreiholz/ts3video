#ifndef VIDEOENCODINGTHREAD_H
#define VIDEOENCODINGTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QPair>
#include <QAtomicInt>
#include <QImage>

#include "vp8frame.h"

class QByteArray;
class QImage;

class VideoEncodingThread : public  QThread
{
	Q_OBJECT

public:
	VideoEncodingThread(QObject* parent);
	~VideoEncodingThread();

	void init(int width, int height, int bitrate = 100, int fps = 24);
	void stop();
	void enqueue(const QImage& image, int senderId);
	void enqueueRecovery(VP8Frame::FrameType ft = VP8Frame::KEY);

protected:
	void run();

signals:
	void error(const QString& message);
	void encoded(QByteArray frame, int senderId);

private:
	QMutex _m;
	QWaitCondition _queueCond;
	QQueue<QPair<QImage, int> > _queue;
	QAtomicInt _stopFlag;
	QAtomicInt _recoveryFlag;

	// Video encoding attributes
	int _width;
	int _height;
	int _bitrate;
	int _fps;
};

#endif