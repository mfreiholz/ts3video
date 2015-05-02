#ifndef VIDEOENCODINGTHREAD_H
#define VIDEOENCODINGTHREAD_H

#include <QThread>
class QByteArray;
class QImage;

#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAtomicInt>
#include <QPair>
#include <QImage>

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

#endif