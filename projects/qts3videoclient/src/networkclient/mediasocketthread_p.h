#ifndef MEDIASOCKETTHREADPRIVATE_H
#define MEDIASOCKETTHREADPRIVATE_H

#include "mediasocketthread.h"
#include <QHostAddress>

class MediaSocketThreadPrivate : public QObject
{
  Q_OBJECT

public:
  MediaSocketThreadPrivate(MediaSocketThread *o) :
    owner(o)
  {}

public:
  MediaSocketThread *owner;
  QSharedPointer<MediaSocket> socket;
  QHostAddress address;
  quint16 port;
  QString token;
};

#endif