#ifndef TS3VIDEOCLIENT_P_H
#define TS3VIDEOCLIENT_P_H

#include <QUdpSocket>

#include "ts3videoclient.h"

class QCorConnection;
class MediaSocket;

class TS3VideoClientPrivate {
  Q_DISABLE_COPY(TS3VideoClientPrivate)
  Q_DECLARE_PUBLIC(TS3VideoClient)
  TS3VideoClientPrivate(TS3VideoClient*);
  TS3VideoClient * const q_ptr;

  QCorConnection *_connection;
  MediaSocket *_mediaSocket;
};

class MediaSocket : public QUdpSocket
{
  Q_OBJECT

public:
  MediaSocket(const QString &token, QObject *parent);
  ~MediaSocket();
  bool authenticated() const;
  void setAuthenticated(bool yesno);

  void sendAuthTokenDatagram(const QString &token);
  void sendVideoFrame(const QByteArray &frameData, quint64 frameId, int senderId);

protected:
  virtual void timerEvent(QTimerEvent *ev);

private slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onReadyRead();

private:
  bool _authenticated;
  QString _token;
  int _authenticationTimerId;

  int _sendVideoFrameTimerId; ///< TODO Remove me (only for dev tests)
};

#endif // TS3VIDEOCLIENT_P_H
