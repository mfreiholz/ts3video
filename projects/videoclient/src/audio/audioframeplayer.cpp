#include "audioframeplayer.h"

AudioFramePlayer::AudioFramePlayer(QObject* parent) :
	QObject(parent), _output(), _outputDevice(0)
{

}

AudioFramePlayer::~AudioFramePlayer()
{
}

QSharedPointer<QAudioOutput> AudioFramePlayer::audioOutput() const
{
	return _output;
}

void AudioFramePlayer::setAudioOutput(const QSharedPointer<QAudioOutput>& out)
{
	_output = out;
	_outputDevice = _output->start();
}

void AudioFramePlayer::add(const PcmFrameRefPtr& f)
{
	if (!_outputDevice || !_outputDevice->isOpen())
		return;
	_outputDevice->write(f->data, f->dataLength());
}