#ifndef VP8FRAME_H
#define VP8FRAME_H

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>
#include <QSharedPointer>

class VP8Frame
{
public:
  enum FrameType
  {
    NORMAL = 0,
    KEY = 1,
    GOLD = 2,
    ALTREF = 3
  };

  VP8Frame();

public:
  quint64 time;
  int type;
  QByteArray data;
};
typedef QSharedPointer<VP8Frame> VP8FrameRefPtr;

QDataStream& operator<<(QDataStream &ds, const VP8Frame &frame);
QDataStream& operator>>(QDataStream &ds, VP8Frame &frame);

#endif