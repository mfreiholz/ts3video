#ifndef AUDIODECODINGTHREAD_H
#define AUDIODECODINGTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>
#include <QQueue>
#include <QPair>
#include "videolib/src/pcmframe.h"
#include "videolib/src/opusframe.h"

class AudioDecodingThread : public QThread
{
	Q_OBJECT

public:
	AudioDecodingThread(QObject* parent);
	~AudioDecodingThread();
	void stop();
	void enqueue(const OpusFrameRefPtr& f, int senderId);

protected:
	void run();

signals:
	void decoded(const PcmFrameRefPtr& f, int senderId);

private:
	QMutex _m;
	QAtomicInt _stopFlag;
	QWaitCondition _queueCond;
	QQueue<QPair<OpusFrameRefPtr, int> > _queue;
};

#endif