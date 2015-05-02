#include "videoencodingthread.h"
#include <QTime>
#include "humblelogging/api.h"
#include "ts3video.h"
#include "vp8frame.h"
#include "../vp8encoder.h"

HUMBLE_LOGGER(HL, "networkclient.videoencodingthread");

VideoEncodingThread::VideoEncodingThread(QObject *parent) :
  QThread(parent),
  _stopFlag(0),
  _recoveryFlag(VP8Frame::NORMAL)
{
}

VideoEncodingThread::~VideoEncodingThread()
{
  stop();
}

void VideoEncodingThread::stop()
{
  _stopFlag = 1;
  _queueCond.wakeAll();
}

void VideoEncodingThread::enqueue(const QImage &image, int senderId)
{
  QMutexLocker l(&_m);
  _queue.enqueue(qMakePair(image, senderId));
  while (_queue.size() > 5) {
    auto item = _queue.dequeue();
  }
  _queueCond.wakeAll();
}

void VideoEncodingThread::enqueueRecovery()
{
  _recoveryFlag = VP8Frame::KEY;
  _queueCond.wakeAll();
}

void VideoEncodingThread::run()
{
  const int fps = 20;
  const int fpsTimeMs = 1000 / fps;
  const int bitRate = 100;
  const QSize dim(IFVS_CLIENT_VIDEO_SIZE);

  QHash<int, VP8Encoder*> encoders;
  QTime fpsTimer;
  fpsTimer.start();

  _stopFlag = 0;
  while (_stopFlag == 0) {
    QMutexLocker l(&_m);
    if (_queue.isEmpty()) {
      _queueCond.wait(&_m);
      continue;
    }
    auto item = _queue.dequeue();
    l.unlock();

    if (item.first.isNull()) {
      continue;
    }

    if (fps > 0 && fpsTimer.elapsed() < fpsTimeMs) {
      continue;
    }
    fpsTimer.restart();

    if (true) {
      // Convert to YuvFrame.
      auto yuv = YuvFrame::fromQImage(item.first);
      // Encode via VPX.
      auto encoder = encoders.value(item.second);
      if (!encoder) {
        HL_DEBUG(HL, QString("Create new VP8 video encoder (id=%1)").arg(item.second).toStdString());
        encoder = new VP8Encoder();
        if (!encoder->initialize(dim.width(), dim.height(), bitRate, fps)) { ///< TODO Find a way to pass this parameters from outside (videoBegin(...), sendVideo(...), videoEnd(...)).
          HL_ERROR(HL, QString("Can not initialize VP8 video encoder").toStdString());
          _stopFlag = 1;
          continue;
        }
        encoders.insert(item.second, encoder);
      }
      if (_recoveryFlag != VP8Frame::NORMAL) {
        encoder->setRequestRecoveryFlag(VP8Frame::KEY);
        _recoveryFlag = VP8Frame::NORMAL;
      }
      auto vp8 = encoder->encode(*yuv);
      delete yuv;
      // Serialize VP8Frame.
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out << *vp8;
      delete vp8;
      emit encoded(data, item.second);
    }

    // DEV Provides plain QImage.
    //if (true) {
    //  static quint64 __frameTime = 1;
    //  VP8Frame vpframe;
    //  vpframe.time = __frameTime++;
    //  vpframe.type = VP8Frame::KEY;
    //  QDataStream out(&vpframe.data, QIODevice::WriteOnly);
    //  out << item.first;

    //  QByteArray data;
    //  QDataStream out2(&data, QIODevice::WriteOnly);
    //  out2 << vpframe;

    //  emit encoded(data, item.second);
    //}

  }
}
