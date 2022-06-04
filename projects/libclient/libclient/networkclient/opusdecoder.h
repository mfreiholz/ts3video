#ifndef OPUSAUDIODECODER_H
#define OPUSAUDIODECODER_H

class OpusFrame;
class PcmFrame;
struct OpusDecoder;

class OpusAudioDecoder
{
public:
	OpusAudioDecoder();
	~OpusAudioDecoder();
	PcmFrame* decode(const OpusFrame& f);

private:
	OpusDecoder* _dec;
};

#endif