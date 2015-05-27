#include "mediasocketthread_p.h"
#include "mediasocket.h"

///////////////////////////////////////////////////////////////////////

MediaSocketThread::MediaSocketThread(const QHostAddress &address, quint16 port, const QString &token, QObject *parent) :
  QThread(parent),
  d(new MediaSocketThreadPrivate(this))
{
  d->address = address;
  d->port = port;
  d->token = token;
}

MediaSocketThread::~MediaSocketThread()
{
}

QSharedPointer<MediaSocket> MediaSocketThread::mediaSocket() const
{
  return d->socket;
}

void MediaSocketThread::run()
{
  QSharedPointer<MediaSocket> mediaSocket(new MediaSocket(d->token, nullptr));
  d->socket = mediaSocket;
  QObject::connect(mediaSocket.data(), &MediaSocket::newVideoFrame, this, &MediaSocketThread::newVideoFrame);
  QObject::connect(mediaSocket.data(), &MediaSocket::networkUsageUpdated, this, &MediaSocketThread::networkUsageUpdated);
  QObject::connect(mediaSocket.data(), &MediaSocket::disconnected, this, &QThread::quit);
  mediaSocket->connectToHost(d->address, d->port);

  exec();
}