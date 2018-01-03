#if defined(OCS_INCLUDE_AUDIO)
#include "opusdecoder.h"
#include "opus_multistream.h"
#include "opus.h"
#include "humblelogging/api.h"
#include <QString>
#include "videolib/src/pcmframe.h"
#include "videolib/src/opusframe.h"

HUMBLE_LOGGER(HL, "opus");

OpusAudioDecoder::OpusAudioDecoder() :
	_dec(nullptr)
{
	auto err = 0;
	auto samplingRate = 8000;
	auto channels = 1;
	_dec = opus_decoder_create(samplingRate, channels, &err);
	if (!_dec || err != OPUS_OK)
	{
		HL_ERROR(HL, QString("Can not initialize opus-encoder (error=%1)").arg(err).toStdString());
	}
}

OpusAudioDecoder::~OpusAudioDecoder()
{
	if (_dec)
	{
		opus_decoder_destroy(_dec);
		_dec = nullptr;
	}
}

PcmFrame* OpusAudioDecoder::decode(const OpusFrame& f)
{
	if (!_dec)
	{
		HL_ERROR(HL, "Decoder is not initialized");
		return nullptr;
	}

	unsigned char* packet = (unsigned char*)f.data.data();
	opus_int32 packetSize = f.data.size();

	//const int minSamples = PcmFrame::calculateNumSamples( 10, 8000 );
	const int maxSamples = PcmFrame::calculateNumSamples(120, 8000);
	opus_int16* decoded = (opus_int16*)malloc(maxSamples * sizeof(opus_int16));

	int samplesRead = opus_decode(_dec, packet, packetSize, decoded, maxSamples, 0);
	if (samplesRead <= 0)
	{
		return nullptr;
	}

	auto pcm = new PcmFrame();
	pcm->data = (char*)malloc(samplesRead * sizeof(opus_int16));
	memcpy(pcm->data, decoded, samplesRead * sizeof(opus_int16));
	pcm->numSamples = samplesRead;
	pcm->numChannels = 1;
	pcm->samplingRate = 8000;

	free(decoded);
	return pcm;
}

#endif
