#ifndef VIDEODECODINGTHREAD_H
#define VIDEODECODINGTHREAD_H

#include <QThread>
#include <QScopedPointer>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QPair>
#include <QAtomicInt>

#include "vp8frame.h"
#include "yuvframe.h"

class VideoDecodingThread : public QThread
{
	Q_OBJECT

public:
	VideoDecodingThread(QObject* parent);
	~VideoDecodingThread();

	void stop();
	void enqueue(VP8Frame* frame, int senderId);

protected:
	void run();

signals:
	void decoded(YuvFrameRefPtr frame, int senderId);

private:
	QMutex _m;
	QWaitCondition _queueCond;
	QQueue<QPair<VP8Frame*, int> > _queue;
	QAtomicInt _stopFlag;
};

#endif