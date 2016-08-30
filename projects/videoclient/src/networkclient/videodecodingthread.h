#ifndef VIDEODECODINGTHREAD_H
#define VIDEODECODINGTHREAD_H

#include <QThread>
#include <QScopedPointer>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QPair>
#include <QAtomicInt>

#include "libbase/defines.h"

#include "videolib/vp8frame.h"
#include "videolib/yuvframe.h"

class VideoDecodingThread : public QThread
{
	Q_OBJECT

public:
	VideoDecodingThread(QObject* parent);
	~VideoDecodingThread();

	void stop();
	void enqueue(VP8Frame* frame, ocs::clientid_t senderId);

protected:
	void run();

signals:
	void decoded(YuvFrameRefPtr frame, ocs::clientid_t senderId);

private:
	QMutex _m;
	QWaitCondition _queueCond;
	QQueue<QPair<VP8Frame*, ocs::clientid_t> > _queue;
	QAtomicInt _stopFlag;
};

#endif