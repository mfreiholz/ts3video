#if defined(OCS_INCLUDE_AUDIO)
#ifndef AUDIOFRAMEPLAYER_H
#define AUDIOFRAMEPLAYER_H

#include <QObject>
#include <QSharedPointer>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include "libapp/src/pcmframe.h"
class QIODevice;

class AudioFramePlayer : public QObject
{
	Q_OBJECT

public:
	AudioFramePlayer(QObject* parent = 0);
	virtual ~AudioFramePlayer();

	QAudioDeviceInfo deviceInfo() const
	{
		return _deviceInfo;
	}
	void setDeviceInfo(const QAudioDeviceInfo& info)
	{
		_deviceInfo = info;
	}

	QAudioFormat format() const
	{
		return _format;
	}
	void setFormat(const QAudioFormat& format)
	{
		_format = format;
	}

	void add(const PcmFrameRefPtr& f, int senderId);

private:
	struct Output
	{
		QSharedPointer<QAudioOutput> out;
		QIODevice* device;
	};
	QHash<int, Output*> _outputs;
	QAudioDeviceInfo _deviceInfo;
	QAudioFormat _format;
};

#endif
#endif