#ifndef VP8FRAME_H
#define VP8FRAME_H

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>

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

	VP8Frame()
		: time(0),
		  type(NORMAL)
	{}

public:
	quint64 time;
	int type;
	QByteArray data;
};

/** Serializes the frame.
 */
QDataStream& operator<<(QDataStream &ds, const VP8Frame &frame)
{
  ds << frame.time << frame.type;
  ds.writeRawData(frame.data.data(), frame.data.size());
  return ds;
}

/** Deserializes the "frame".
 * The frame's data QByteArray must have the correct size.
 * Use QByteArray::resize() before to allocate the required memory.
 */
QDataStream& operator>>(QDataStream &ds, VP8Frame &frame)
{
  ds >> frame.time;
  ds >> frame.type;
  ds.readRawData(frame.data.data(), frame.data.size());
  return ds;
}

#endif