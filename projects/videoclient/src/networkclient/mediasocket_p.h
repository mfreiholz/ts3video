#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"

#include "videolib/networkusageentity.h"

#include "udpvideoframedecoder.h"
#include "videoencodingthread.h"
#include "videodecodingthread.h"

#if defined(OCS_INCLUDE_AUDIO)
#include "audioencodingthread.h"
#include "audiodecodingthread.h"
#include "audioudpdecoder.h"
#endif

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
#if defined(OCS_INCLUDE_AUDIO)
		audioEncodingThread(new AudioEncodingThread(this)),
		audioDecodingThread(new AudioDecodingThread(this)),
#endif
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
	QHash<ocs::clientid_t, VideoFrameUdpDecoder*> videoFrameDatagramDecoders;  ///< Maps client-id to it's decoder.
	VideoDecodingThread* videoDecodingThread;

#if defined(OCS_INCLUDE_AUDIO)
	// AUDIO
	// Encoding
	AudioEncodingThread* audioEncodingThread;

	// Decoding
	QHash<ocs::clientid_t, AudioUdpDecoder*> audioFrameDatagramDecoders;
	AudioDecodingThread* audioDecodingThread;
#endif

	// STATISTICS

	// Network usage.
	NetworkUsageEntity networkUsage;
	NetworkUsageEntityHelper networkUsageHelper;
};

#endif