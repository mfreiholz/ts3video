#include "vp8frame.h"

VP8Frame::VP8Frame() :
  time(0),
  type(NORMAL)
{}

QDataStream& operator<<(QDataStream &ds, const VP8Frame &frame)
{
  ds << frame.time << frame.type;
  //ds.writeRawData(frame.data.data(), frame.data.size());
  ds << frame.data;
  return ds;
}

QDataStream& operator>>(QDataStream &ds, VP8Frame &frame)
{
  ds >> frame.time;
  ds >> frame.type;
  ds >> frame.data;
  //ds.readRawData(frame.data.data(), frame.data.size());
  return ds;
}