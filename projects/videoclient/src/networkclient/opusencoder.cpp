#if defined(OCS_INCLUDE_AUDIO)
#include "opusencoder.h"
#include "opus.h"
#include "opus_multistream.h"
#include "humblelogging/api.h"
#include "libapp/src/pcmframe.h"
#include "libapp/src/opusframe.h"
#include <QString>

HUMBLE_LOGGER(HL, "opus");

OpusAudioEncoder::OpusAudioEncoder() :
	_enc(nullptr),
	_nextFrameId(0)
{
	auto err = 0;
	auto samplingRate = 8000;
	auto channels = 1;
	_enc = opus_encoder_create(samplingRate, channels, OPUS_APPLICATION_VOIP, &err);
	if (!_enc || err != OPUS_OK)
	{
		HL_ERROR(HL, QString("Can not initialize opus-encoder (error=%1)").arg(err).toStdString());
	}
}

OpusAudioEncoder::~OpusAudioEncoder()
{
	if (_enc)
	{
		opus_encoder_destroy(_enc);
		_enc = nullptr;
	}
}

OpusFrame* OpusAudioEncoder::encode(const PcmFrame& f)
{
	if (!_enc)
	{
		return NULL;
	}

	opus_int16* audioFrame = (opus_int16*)f.data; // Input audio data.
	int frameSize = f.numSamples; // Duration of the frame in samples (per channel).
	opus_int32 maxPacket = OPUS_MAX_PACKET; // Maximum number of bytes that can be written in the packet (4000 bytes is recommended).
	unsigned char* data = (unsigned char*)malloc(OPUS_MAX_PACKET); // Output byte array to which the compressed data is written.

	opus_int32 len = opus_encode(_enc, audioFrame, frameSize, data, maxPacket);
	QByteArray encodedData((char*)data, len);
	free(data);

	OpusFrame* opusFrame = new OpusFrame();
	opusFrame->time = ++_nextFrameId;
	opusFrame->data = encodedData;
	return opusFrame;
}

#endif
