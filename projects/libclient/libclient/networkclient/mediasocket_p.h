#ifndef MEDIASOCKETPRIVATE_H
#define MEDIASOCKETPRIVATE_H

#include "mediasocket.h"

#include "libapp/networkusageentity.h"

#include "udpvideoframedecoder.h"
#include "videoencodingthread.h"
#include "videodecodingthread.h"

#if defined(OCS_INCLUDE_AUDIO)
#include "audioencodingthread.h"
#include "audiodecodingthread.h"
#include "audioudpdecoder.h"
#endif

#include <QCache>

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
		videoFrameCache(0/*1024 * 32*/),
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
	QHash<ocs::clientid_t, VideoFrameUdpDecoder*>
	videoFrameDatagramDecoders;  ///< Maps client-id to it's decoder.
	VideoDecodingThread* videoDecodingThread;

	QCache<UDP::VideoFrameDatagram::dg_frame_id_t, QByteArray>
	videoFrameCache;

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