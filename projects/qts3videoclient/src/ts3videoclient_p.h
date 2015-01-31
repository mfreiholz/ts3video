#ifndef TS3VIDEOCLIENT_P_H
#define TS3VIDEOCLIENT_P_H

#include <QUdpSocket>

#include "cliententity.h"
#include "channelentity.h"
#include "jsonprotocolhelper.h"

#include "ts3videoclient.h"

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
  VideoEncodingThread *_encodingThread;
};

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

protected:
  virtual void timerEvent(QTimerEvent *ev);

private slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onReadyRead();

private:
  bool _authenticated;
  QString _token;
  int _authenticationTimerId;
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

#endif // TS3VIDEOCLIENT_P_H
