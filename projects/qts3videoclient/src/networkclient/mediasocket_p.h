#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"

class MediaSocketPrivate : public QObject
{
  Q_OBJECT

public:
  MediaSocketPrivate(MediaSocket *o) :
    owner(o)
  {}

public:
  MediaSocket *owner;
};

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAtomicInt>
#include "yuvframe.h"
#include "vp8frame.h"
class QByteArray;
class QImage;

/*!
  Encodes and serializes video frames.
*/
class VideoEncodingThread : public  QThread
{
  Q_OBJECT

public:
  VideoEncodingThread(QObject *parent);
  ~VideoEncodingThread();
  void stop();
  void enqueue(const QImage &image, int senderId);
  void enqueueRecovery();

protected:
  void run();

signals:
  void encoded(QByteArray &frame, int senderId);

private:
  QMutex _m;
  QWaitCondition _queueCond;
  QQueue<QPair<QImage, int> > _queue; ///< Replace with RingQueue (Might not keep more than X frames! Otherwise we might get a memory problem.)
  QAtomicInt _stopFlag;
  QAtomicInt _recoveryFlag;
};

/*!
*/
class VideoDecodingThread : public QThread
{
  Q_OBJECT

public:
  VideoDecodingThread(QObject *parent);
  ~VideoDecodingThread();
  void stop();
  void enqueue(VP8Frame *frame, int senderId);

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
