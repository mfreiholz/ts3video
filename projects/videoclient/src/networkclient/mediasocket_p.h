#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"
#include "networkusageentity.h"

#include "udpvideoframedecoder.h"
#include "videoencodingthread.h"
#include "videodecodingthread.h"

#include "audioencodingthread.h"
#include "audiodecodingthread.h"
#include "audioudpdecoder.h"

class MediaSocketPrivate : public QObject
{
	Q_OBJECT

public:
	MediaSocketPrivate(MediaSocket* o) :
		owner(o),
		authenticated(false),
		authenticationTimerId(-1),
		keepAliveTimerId(-1),
		videoEncodingThread(new VideoEncodingThread(this)),
		lastFrameRequestTimestamp(0),
		videoDecodingThread(new VideoDecodingThread(this)),
		audioEncodingThread(new AudioEncodingThread(this)),
		audioDecodingThread(new AudioDecodingThread(this)),
		networkUsage(),
		networkUsageHelper(networkUsage)
	{}

public:
	MediaSocket* owner;

	bool authenticated;
	QString token;
	int authenticationTimerId;
	int keepAliveTimerId;

	// VIDEO
	
	// Encoding
	VideoEncodingThread* videoEncodingThread;
	unsigned long long lastFrameRequestTimestamp;
	
	// Decoding
	QHash<int, VideoFrameUdpDecoder*> videoFrameDatagramDecoders;  ///< Maps client-id to it's decoder.
	VideoDecodingThread* videoDecodingThread;

	// AUDIO
	// Encoding
	AudioEncodingThread* audioEncodingThread;

	// Decoding
	QHash<int, AudioUdpDecoder*> audioFrameDatagramDecoders;
	AudioDecodingThread* audioDecodingThread;

	// STATISTICS

	// Network usage.
	NetworkUsageEntity networkUsage;
	NetworkUsageEntityHelper networkUsageHelper;
};

#endif