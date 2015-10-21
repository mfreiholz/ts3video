#ifndef OPUSFRAME_H
#define OPUSFRAME_H

#include <QtGlobal>
#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include <QDataStream>
class QIODevice;
class QDataStream;

class OpusFrame
{
public:
	enum FrameType
	{
		NORMAL = 0
	};

	OpusFrame();
	~OpusFrame();

public:
	quint64 time;
	int type;
	QByteArray data;
};
typedef QSharedPointer<OpusFrame> OpusFrameRefPtr;

QDataStream& operator<<(QDataStream &ds, const OpusFrame &frame);
QDataStream& operator>>(QDataStream &ds, OpusFrame &frame);

#endif