#ifndef PCMFRAME_H
#define PCMFRAME_H

#include <QSharedPointer>

class PcmFrame
{
public:
	PcmFrame();
	~PcmFrame();

	PcmFrame* copy() const;
	int dataLength() const;
	float frameLength() const;

	static float calculateFrameLength(int numSamples, int samplingRate);
	static int calculateNumSamples(int length, int samplingRate);

public:
	char* data;
	int numSamples;
	int numChannels;
	int samplingRate;
};
typedef QSharedPointer<PcmFrame> PcmFrameRefPtr;

#endif