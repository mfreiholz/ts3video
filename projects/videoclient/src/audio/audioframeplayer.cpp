#if defined(OCS_INCLUDE_AUDIO)
#include "audioframeplayer.h"

AudioFramePlayer::AudioFramePlayer(QObject* parent) :
	QObject(parent)
{
}

AudioFramePlayer::~AudioFramePlayer()
{
	qDeleteAll(_outputs);
}

void AudioFramePlayer::add(const PcmFrameRefPtr& f, int senderId)
{
	auto out = _outputs.value(senderId);
	if (!out)
	{
		out = new Output();
		out->out = QSharedPointer<QAudioOutput>(new QAudioOutput(_deviceInfo, _format));
		out->device = out->out->start();
		_outputs.insert(senderId, out);
	}
	if (!out || !out->device || !out->device || !out->device->isOpen())
	{
		return;
	}
	out->device->write(f->data, f->dataLength());
}

#endif
