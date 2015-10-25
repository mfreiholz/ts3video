#ifndef AUDIOFRAMEPLAYER_H
#define AUDIOFRAMEPLAYER_H

#include <QObject>
#include <QSharedPointer>
#include <QAudioOutput>
#include "videolib/src/pcmframe.h"
class QIODevice;

class AudioFramePlayer : public QObject
{
	Q_OBJECT

public:
	AudioFramePlayer(QObject* parent = 0);
	virtual ~AudioFramePlayer();

	QSharedPointer<QAudioOutput> audioOutput() const;
	void setAudioOutput(const QSharedPointer<QAudioOutput>& out);

	void add(const PcmFrameRefPtr& f);

private:
	QSharedPointer<QAudioOutput> _output;
	QIODevice* _outputDevice;
};

#endif