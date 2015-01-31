#ifndef TS3VIDEOCLIENT_P_H
#define TS3VIDEOCLIENT_P_H

#include <QHash>
#include <QPair>
#include <QUdpSocket>

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

#include "vp8frame.h"

#include "ts3videoclient.h"
#include "udpvideoframedecoder.h"

class QCorConnection;
class MediaSocket;
class VideoEncodingThread;

class TS3VideoClientPrivate {
  Q_DISABLE_COPY(TS3VideoClientPrivate)
  Q_DECLARE_PUBLIC(TS3VideoClient)
  TS3VideoClientPrivate(TS3VideoClient*);
  TS3VideoClient * const q_ptr;

  QCorConnection *_connection;
  MediaSocket *_mediaSocket;
  ClientEntity _clientEntity;

  // Encoding.
  // TODO Move into MediaSocket.
  VideoEncodingThread *_encodingThread;
};

/*!
  Handles media data over UDP.
*/
class MediaSocket : public QUdpSocket
{
  Q_OBJECT

public:
  MediaSocket(const QString &token, QObject *parent);
  ~MediaSocket();
  bool isAuthenticated() const;
  void setAuthenticated(bool yesno);
  void sendAuthTokenDatagram(const QString &token);
  void sendVideoFrame(const QByteArray &frame, quint64 frameId, quint32 senderId);

signals:
  void newVideoFrame(const QImage &image, int senderId);

protected:
  virtual void timerEvent(QTimerEvent *ev);

private slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onReadyRead();

private:
  bool _authenticated;
  QString _token;
  int _authenticationTimerId;

  // Decoding.
  QHash<int, VideoFrameUdpDecoder*> _videoFrameDatagramDecoders; ///< Maps client-id to it's decoder.
  class VideoDecodingThread *_videoDecodingThread;
};

/*!
  Encodes and serializes video frames.
 */
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
class VideoEncodingThread : public  QThread
{
  Q_OBJECT

public:
  VideoEncodingThread(QObject *parent);
  ~VideoEncodingThread();
  void stop();
  void enqueue(const QImage &image);

protected:
  void run();

signals:
  void newEncodedFrame(QByteArray &frame);

private:
  QMutex _m;
  QWaitCondition _queueCond;
  QQueue<QImage> _queue; ///< Replace with RingQueue (Might not keep more than X frames! Otherwise we might get a memory problem.)
  QAtomicInt _stopFlag;
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
  void enqueue(int senderId, VP8Frame *frame);

protected:
  void run();

signals:
  void decoded(const QImage &image, int senderId);

private:
  QMutex _m;
  QWaitCondition _queueCond;
  QQueue<QPair<int, VP8Frame*> > _queue;
  QAtomicInt _stopFlag;
};

#endif // TS3VIDEOCLIENT_P_H
