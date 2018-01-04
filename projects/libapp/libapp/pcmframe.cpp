#include "pcmframe.h"

PcmFrame::PcmFrame() :
	data(0), numSamples(0), numChannels(0), samplingRate(0)
{
}

PcmFrame::~PcmFrame()
{
	if (data)
	{
		free(data);
	}
}

PcmFrame* PcmFrame::copy() const
{
	auto f = new PcmFrame;
	f->numSamples = numSamples;
	f->numChannels = numChannels;
	if (data)
	{
		f->data = (char*)malloc(sizeof(char) * numSamples * numChannels * 2);
		memcpy(f->data, data, sizeof(char) * numSamples * numChannels * 2);
	}
	return f;
}

int PcmFrame::dataLength() const
{
	return numSamples * numChannels * 2;
}

// frameLength calculates the frame size in milliseconds.
float PcmFrame::frameLength() const
{
	return PcmFrame::calculateFrameLength(numSamples, samplingRate);
}

// calculateFrameLength calculates the frame size in milliseconds.
float PcmFrame::calculateFrameLength(int numSamples, int samplingRate)
{
	float framesize = (float)numSamples / (float)samplingRate / 1000.0f;
	return framesize;
}

int PcmFrame::calculateNumSamples(int length, int samplingRate)
{
	int numSamples = length * samplingRate / 1000;
	return numSamples;
}
