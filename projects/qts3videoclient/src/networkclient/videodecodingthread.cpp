#include "videodecodingthread.h"
#include "humblelogging/api.h"
#include "ts3video.h"
#include "../vp8decoder.h"

HUMBLE_LOGGER(HL, "networkclient.videodecodingthread");

VideoDecodingThread::VideoDecodingThread(QObject *parent) :
  QThread(parent)
{

}

VideoDecodingThread::~VideoDecodingThread()
{
  stop();
}

void VideoDecodingThread::stop()
{
  _stopFlag = 1;
  _queueCond.wakeAll();
}

void VideoDecodingThread::enqueue(VP8Frame *frame, int senderId)
{
  QMutexLocker l(&_m);
  _queue.enqueue(qMakePair(frame, senderId));
  //while (_queue.size() > 5) {
  //  auto item = _queue.dequeue();
  //  delete item.first;
  //}
  _queueCond.wakeAll();
}

void VideoDecodingThread::run()
{
  QHash<int, VP8Decoder*> decoders;

  _stopFlag = 0;
  while (_stopFlag == 0) {
    QMutexLocker l(&_m);
    if (_queue.isEmpty()) {
      _queueCond.wait(&_m);
      continue;
    }
    auto item = _queue.dequeue();
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
}