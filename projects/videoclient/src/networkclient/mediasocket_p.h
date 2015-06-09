#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"
#include "networkusageentity.h"
#include "udpvideoframedecoder.h"
#include "videoencodingthread.h"
#include "videodecodingthread.h"

class MediaSocketPrivate : public QObject
{
  Q_OBJECT

public:
  MediaSocketPrivate(MediaSocket *o) :
    owner(o),
    authenticated(false),
    authenticationTimerId(-1),
    videoEncodingThread(new VideoEncodingThread(this)),
    lastFrameRequestTimestamp(0),
    videoDecodingThread(new VideoDecodingThread(this)),
    networkUsage(),
    networkUsageHelper(networkUsage)
  {}

public:
  MediaSocket *owner;

  bool authenticated;
  QString token;
  int authenticationTimerId;

  // Encoding.
  VideoEncodingThread *videoEncodingThread;
  unsigned long long lastFrameRequestTimestamp;

  // Decoding.
  QHash<int, VideoFrameUdpDecoder *> videoFrameDatagramDecoders; ///< Maps client-id to it's decoder.
  VideoDecodingThread *videoDecodingThread;

  // Network usage.
  NetworkUsageEntity networkUsage;
  NetworkUsageEntityHelper networkUsageHelper;
};

#endif