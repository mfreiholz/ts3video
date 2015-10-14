#ifndef OPUSENCODER_H
#define OPUSENCODER_H

#include <QtGlobal>
class OpusFrame;
class PcmFrame;
struct OpusEncoder;

#define OPUS_MAX_PACKET (4000)

class OpusAudioEncoder
{
public:
	OpusAudioEncoder();
	~OpusAudioEncoder();
	OpusFrame* encode(const PcmFrame& f);

private:
	OpusEncoder* _enc;
	qint64 _nextFrameId;
};

#endif