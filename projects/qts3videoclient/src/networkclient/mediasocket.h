#ifndef MEDIASOCKET_H
#define MEDIASOCKET_H

#include <QScopedPointer>
#include <QUdpSocket>
#include "yuvframe.h"
#include "networkusageentity.h"
#include "../udpvideoframedecoder.h"

class MediaSocketPrivate;
class MediaSocket : public QUdpSocket
{
  Q_OBJECT
  friend class MediaSocketPrivate;
  QScopedPointer<MediaSocketPrivate> d;

public:
  MediaSocket(const QString &token, QObject *parent);
  ~MediaSocket();

  bool isAuthenticated() const;
  void setAuthenticated(bool yesno);

  /*! Encodes and sends the image as new video frame of the current
      video stream to the server.
  */
  void sendVideoFrame(const QImage &image, int senderId);

signals:
  /*! Emits with every new arrived and decoded video frame.
  */
  void newVideoFrame(YuvFrameRefPtr frame, int senderId);

  /*! Emits periodically with newest calculated network-usage information.
  */
  void networkUsageUpdated(const NetworkUsageEntity &networkUsage);

protected:
  void sendAuthTokenDatagram(const QString &token);
  void sendVideoFrame(const QByteArray &frame, quint64 frameId, quint32 senderId);
  void sendVideoFrameRecoveryDatagram(quint64 frameId, quint32 fromSenderId);
  virtual void timerEvent(QTimerEvent *ev);

  private slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onReadyRead();

private:
  bool _authenticated;
  QString _token;
  int _authenticationTimerId;

  // Encoding.
  class VideoEncodingThread *_videoEncodingThread;
  unsigned long long _lastFrameRequestTimestamp;

  // Decoding.
  QHash<int, VideoFrameUdpDecoder*> _videoFrameDatagramDecoders; ///< Maps client-id to it's decoder.
  class VideoDecodingThread *_videoDecodingThread;

  // Network usage.
  NetworkUsageEntity _networkUsage;
  NetworkUsageEntityHelper _networkUsageHelper;
};


#endif