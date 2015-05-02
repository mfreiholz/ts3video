#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"

class MediaSocketPrivate : public QObject
{
  Q_OBJECT

public:
  MediaSocketPrivate(MediaSocket *o) :
    owner(o)
  {}

public:
  MediaSocket *owner;
};

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAtomicInt>
#include "yuvframe.h"
#include "vp8frame.h"
class QByteArray;
class QImage;



#endif
