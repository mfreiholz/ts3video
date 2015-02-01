#ifndef _VP8ENCODER_HEADER_
#define _VP8ENCODER_HEADER_

#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

#include "yuvframe.h"

class VP8Frame;

void rgbToYV12( unsigned char *pRGBData, int nFrameWidth, int nFrameHeight, void *pFullYPlane, void *pDownsampledUPlane, void *pDownsampledVPlane );

/*!
  This class encodes raw video into VP8 encoded video data.

  \note This class is NOT thread-safe.
*/
class VP8Encoder
{
public:
  VP8Encoder();
  ~VP8Encoder();

  /*!
    Initializes the encoder to work with the given frame geometry.
    All incoming raw frames have to be in exactly the configured size,
    otherwise the encoding will fail.

    \param[in] width
      Width of the video frames.
    \param[in] height
      Height of the video frames.
    \param[in] bitrate
      The average bitrate which the encoder should try to use.
    \param[in] framerate
      Frame rate of the video.
  */
  bool initialize(int width, int height, int bitrate, int framerate);

  /*!
    Encodes the given raw frame with VP8.

    \param[in] frame
      The raw frame which should be encoded.

    \return Pointer to a VP8Frame object or NULL, if an error occured.
      The caller takes ownership of the returning object.
  */
  VP8Frame* encode(YuvFrame &frame);

  /*!
    Sets a temporary flag for the next call to "encode",
    which lets the encoder create a specific type of frame.

    \param[in] recoveryFlag (VP8Frame::FrameType)
  */
  void setRequestRecoveryFlag(int recoveryFlag);

private:
  vpx_codec_ctx_t     _codec;
  vpx_codec_enc_cfg_t _cfg;
  vpx_image_t         _raw;
  unsigned long       _incremental_frame_number;
  int _width;
  int _height;
  int _request_recovery_flag;
};

#endif