#include "opusframe.h"

OpusFrame::OpusFrame() :
	time(0), type(NORMAL), data()
{
}

OpusFrame::~OpusFrame()
{
}

QDataStream& operator<<(QDataStream &ds, const OpusFrame &frame)
{
	ds << frame.time;
	ds << frame.type;
	ds << frame.data;
	return ds;
}

QDataStream& operator>>(QDataStream &ds, OpusFrame &frame)
{
	ds >> frame.time;
	ds >> frame.type;
	ds >> frame.data;
	return ds;
}