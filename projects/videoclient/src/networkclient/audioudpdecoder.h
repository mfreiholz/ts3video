#if defined(OCS_INCLUDE_AUDIO)
#ifndef AUDIOUDPDECODER_H
#define AUDIOUDPDECODER_H

#include <vector>
#include <map>

#include "libmediaprotocol/include/medprotocol.h"
#include "libapp/src/opusframe.h"

class AudioUdpDecoder
{
public:
	enum ErrorType
	{
		NoError,
		InvalidParameterError,
		AlreadyProcessedFrameError
	};

	AudioUdpDecoder();
	~AudioUdpDecoder();
	int add(UDP::AudioFrameDatagram* datagram);
	OpusFrame* next();

protected:
	struct FrameBufferValue
	{
		unsigned long long firstReceivedDatagramTimestamp;
		std::vector<UDP::AudioFrameDatagram*> datagrams;
	};

	bool isComplete(const std::vector<UDP::AudioFrameDatagram*>& frameBuffer) const;
	OpusFrame* createFrame(const std::vector<UDP::AudioFrameDatagram*>& frameBuffer) const;
	void checkFrameBuffers(unsigned int maxSize = 16);
	void checkCompletedFramesQueue(unsigned int maxSize = 16);

private:
	int _lastError;
	std::map<unsigned long long, struct FrameBufferValue*> _frameBuffers;
	std::map<unsigned long long, OpusFrame*> _completedFramesQueue;
	unsigned long long _lastCompletedFrameId;
};

#endif
#endif
