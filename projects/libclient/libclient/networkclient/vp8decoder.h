#ifndef _VP8DECODER_HEADER_
#define _VP8DECODER_HEADER_

#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"

class QByteArray;
class QImage;
class YuvFrame;

/*!
	\class VP8Decoder
	This class decodes VP8 video data into RGB frames.
*/
class VP8Decoder
{
public:
	VP8Decoder();
	virtual ~VP8Decoder();

	void initialize();
	QImage decodeFrame(const QByteArray& frame);
	YuvFrame* decodeFrameRaw(const QByteArray& frame);

private:
	vpx_codec_ctx_t _codec;
	int _frameCount;
};

#endif