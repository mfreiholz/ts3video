#ifndef VIDEOENCODINGTHREAD_H
#define VIDEOENCODINGTHREAD_H

#include <QThread>
#include <QScopedPointer>
class QByteArray;
class QImage;

class VideoEncodingThreadPrivate;
class VideoEncodingThread : public  QThread
{
  Q_OBJECT
  friend class VideoEncodingThreadPrivate;
  QScopedPointer<VideoEncodingThreadPrivate> d;

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
};

#endif