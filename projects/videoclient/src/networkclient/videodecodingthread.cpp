#include "videodecodingthread.h"
#include "humblelogging/api.h"
#include "ts3video.h"
#include "../vp8decoder.h"

HUMBLE_LOGGER(HL, "networkclient.videodecodingthread");

///////////////////////////////////////////////////////////////////////

#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QPair>
#include <QAtomicInt>

class VideoDecodingThreadPrivate
{
public:
  VideoDecodingThreadPrivate(VideoDecodingThread *o) :
    owner(o)
  {}

public:
  VideoDecodingThread *owner;
  QMutex m;
  QWaitCondition queueCond;
  QQueue<QPair<VP8Frame*, int> > queue;
  QAtomicInt stopFlag;
};

///////////////////////////////////////////////////////////////////////

VideoDecodingThread::VideoDecodingThread(QObject *parent) :
  QThread(parent),
  d(new VideoDecodingThreadPrivate(this))
{
}

VideoDecodingThread::~VideoDecodingThread()
{
  stop();
  wait();
  while (!d->queue.isEmpty()) {
    auto item = d->queue.dequeue();
    delete item.first;
  }
}

void VideoDecodingThread::stop()
{
  d->stopFlag = 1;
  d->queueCond.wakeAll();
}

void VideoDecodingThread::enqueue(VP8Frame *frame, int senderId)
{
  QMutexLocker l(&d->m);
  d->queue.enqueue(qMakePair(frame, senderId));
  //while (_queue.size() > 5) {
  //  auto item = _queue.dequeue();
  //  delete item.first;
  //}
  d->queueCond.wakeAll();
}

void VideoDecodingThread::run()
{
  // TODO Make it possible to reset/delete a VP8Decoder.
  //      That would be useful when a participant leaves.
  QHash<int, VP8Decoder*> decoders;

  d->stopFlag = 0;
  while (d->stopFlag == 0) {
    QMutexLocker l(&d->m);
    if (d->queue.isEmpty()) {
      d->queueCond.wait(&d->m);
      continue;
    }
    auto item = d->queue.dequeue();
    l.unlock();

    if (!item.first || item.second == 0) {
      continue;
    }

    if (true) {
      // Decode VPX frame to YuvFrame.
      auto decoder = decoders.value(item.second);
      if (!decoder) {
        decoder = new VP8Decoder();
        decoder->initialize();
        decoders.insert(item.second, decoder);
      }
      auto yuv = YuvFrameRefPtr(decoder->decodeFrameRaw(item.first->data));
      //auto image = yuv->toQImage(); ///< TODO This call is VERY instense! We may want to work with YuvFrame's directly.
      emit decoded(yuv, item.second);
    }

    // DEV
    //if (true) {
    //  QImage image;
    //  QDataStream in(item.first->data);
    //  in >> image;
    //  delete item.first;
    //  emit decoded(image, item.second);
    //}

  }

  // Clean up.
  qDeleteAll(decoders);
}