#ifndef VIDEODECODINGTHREAD_H
#define VIDEODECODINGTHREAD_H

#include <QThread>
#include <QScopedPointer>
#include "vp8frame.h"
#include "yuvframe.h"

class VideoDecodingThreadPrivate;
class VideoDecodingThread : public QThread
{
	Q_OBJECT
	friend class VideoDecodingThreadPrivate;
	QScopedPointer<VideoDecodingThreadPrivate> d;

public:
	VideoDecodingThread(QObject* parent);
	~VideoDecodingThread();
	void stop();
	void enqueue(VP8Frame* frame, int senderId);

protected:
	void run();

signals:
	void decoded(YuvFrameRefPtr frame, int senderId);
};

#endif