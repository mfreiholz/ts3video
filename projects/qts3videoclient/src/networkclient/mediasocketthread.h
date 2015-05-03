#ifndef MEDIASOCKETTHREAD_H
#define MEDIASOCKETTHREAD_H

#include <QThread>
#include <QScopedPointer>
#include <QSharedPointer>
#include "yuvframe.h"
class QHostAddress;
class MediaSocket;
class NetworkUsageEntity;

class MediaSocketThreadPrivate;
class MediaSocketThread : public QThread
{
  Q_OBJECT
  friend class MediaSocketThreadPrivate;
  QScopedPointer<MediaSocketThreadPrivate> d;

public:
  MediaSocketThread(const QHostAddress &address, quint16 port, const QString &token, QObject *parent = nullptr);
  virtual ~MediaSocketThread();

  QSharedPointer<MediaSocket> mediaSocket() const;

protected:
  void run();

signals:
  void newVideoFrame(YuvFrameRefPtr frame, int senderId);
  void networkUsageUpdated(const NetworkUsageEntity &networkUsage);
};

#endif