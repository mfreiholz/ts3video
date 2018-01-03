#if defined(OCS_INCLUDE_AUDIO)
#ifndef AUDIOENCODINGTHREAD_H
#define AUDIOENCODINGTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>
#include <QQueue>
#include <QPair>
#include "videolib/src/pcmframe.h"
#include "videolib/src/opusframe.h"

class AudioEncodingThread : public QThread
{
	Q_OBJECT

public:
	AudioEncodingThread(QObject* parent);
	~AudioEncodingThread();

	void stop();
	void enqueue(const PcmFrameRefPtr& f, int senderId);
	void enqueueRecovery();

protected:
	void run();

signals:
	void encoded(const QByteArray& f, int senderId);

private:
	QMutex _m;
	QWaitCondition _queueCond;
	QQueue<QPair<PcmFrameRefPtr, int> > _queue; ///< Replace with RingQueue (Might not keep more than X frames! Otherwise we might get a memory problem.)
	QAtomicInt _stopFlag;
	QAtomicInt _recoveryFlag;
};

#endif
#endif
