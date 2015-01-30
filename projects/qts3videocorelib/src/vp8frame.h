#ifndef VP8FRAME_H
#define VP8FRAME_H

#include <QtGlobal>
#include <QByteArray>

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

#endif