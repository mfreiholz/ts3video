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
  MediaSocket(QObject *parent);
  ~MediaSocket();

private:
  bool _authenticated;
};

#endif // TS3VIDEOCLIENT_P_H
