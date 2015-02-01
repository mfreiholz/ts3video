#include "vp8decoder.h"

#include <QtMath>
#include <QDebug>

#include "yuvframe.h"
#include "vp8frame.h"
#include "imageutil.h"

// VPX defines.
#define VPX_CODEC_DISABLE_COMPAT 1
#define vpxinterface (vpx_codec_vp8_dx())
#define fourcc 0x30385056
#define IVF_FILE_HDR_SZ (32)
#define IVF_FRAME_HDR_SZ (12)


static unsigned int qbaFromLe32( const QByteArray &arr, int offset = 0 )
{
  unsigned int n = ( arr.at(offset+3) << 24 | arr.at(offset+2) << 16 | arr.at(offset+1) << 8 | arr.at(offset) );
  return n;
}


static unsigned int mem_get_le32( const unsigned char *mem ) {
  return (mem[3] << 24)|(mem[2] << 16)|(mem[1] << 8)|(mem[0]);
}


static unsigned char* extractData( const QByteArray &arr, int offset, int length )
{
  if( arr.length() - offset < length )
    return NULL;

  unsigned char *ret = (unsigned char*)malloc( sizeof( unsigned char ) * length );
  for( int i = 0; i < length; i++ )
  {
    ret[i] = arr.at( offset + i );
  }

  return ret;
}


/*****************************************************************************/
/* VP8Decoder                                                                */
/*****************************************************************************/

VP8Decoder::VP8Decoder( QObject *parent )
  : QObject( parent )
  , _codec()
  , _frames( 0 )
  , _raw()
  , _flags( 0 )
  , _width( 0 )
  , _height( 0 )
  , _stop( true )
  , _mutex()
{
}


vpx_codec_ctx_t & VP8Decoder::getCodec()
{
  return _codec;
}


int VP8Decoder::getFrameCount()
{
  return _frames;
}


void VP8Decoder::initialize()
{
  vpx_codec_err_t res;

  /* Initialize codec */
  res = vpx_codec_dec_init( &_codec, vpxinterface, NULL, _flags );
  if(res) {
    const QString msg( "Failed to initialize encoder" );
    emit error( msg );
    return;
  }
  
  // Enable encoding.
  _stop = false;
  _frames = 0;
}


QImage VP8Decoder::decodeFrame( const QByteArray &data )
{
  // Read frame size from header.
  const uchar *header = extractData( data, 0, 4 );
  size_t frame_sz = mem_get_le32( header );
  delete header;
  vpx_codec_iter_t iter = NULL;
  vpx_image_t *img;

  const uint8_t *frame = extractData( data, IVF_FRAME_HDR_SZ, frame_sz );
  if( !frame ) // Stuff went horribly wrong.
  {
    const QString msg( "Failed to read frame" );
    emit error( msg );
    return QImage();
  }
  
  _frames++;
 
  /* Decode the frame */
  if( vpx_codec_decode( &_codec, frame, frame_sz, NULL, 0 ) )
  {
    const QString msg( "Failed to decode frame: %1 - %2" );
    const char* err = vpx_codec_error( &_codec );
    const char* errdetail = vpx_codec_error_detail( &_codec );
    emit error( msg.arg( err ).arg( errdetail ) );
    return QImage();
  }
 
  /* Convert decoded data */
  uint8_t *out = NULL;
  if( (img = vpx_codec_get_frame( &_codec, &iter )) ) { // <-------------------------------- Originally while-loop
    uint8_t *rgb = (unsigned char*)malloc( 3 * img->d_w * img->d_h );
    out = rgb;
    
    // First plane from codec is 1:1 Y (luminance) data.
    uint8_t *py = img->planes[0];
    
    // Second and third planes are 1:2 subsampled U/V.
    uint8_t *tpu = (uint8_t*)malloc( img->d_w * img->d_h );
    uint8_t *pu = tpu;
    uint8_t *tpv = (uint8_t*)malloc( img->d_w * img->d_h );
    uint8_t *pv = tpv;
    
    // High quality 1:2 upsampling of U and V using Lanczos filter.
    lanczos_interp2(img->planes[1], pu, img->stride[1], img->d_w, img->d_h);
    lanczos_interp2(img->planes[2], pv, img->stride[2], img->d_w, img->d_h);
    
    for( uint j = 0; j < img->d_h; ++j){
      for( uint i = 0; i < img->d_w; ++i){
        int y = py[i] - 16;
        int u = pu[i] - 128;
        int v = pv[i] - 128;
        rgb[0] = clamp(SCALEYUV(rcoeff(y, u, v)));
        rgb[1] = clamp(SCALEYUV(gcoeff(y, u, v)));
        rgb[2] = clamp(SCALEYUV(bcoeff(y, u, v)));
        rgb += 3;
      }
      py += img->stride[0];
      pu += img->d_w;
      pv += img->d_w;
    }

    // Clean-up.
    delete tpu;
    delete tpv;
  }

  if( frame )
    delete frame;

  if( out )
  {
    QImage *qimg = new QImage( out, img->d_w, img->d_h, QImage::Format_RGB888 );
    QImage ret = qimg->copy();
    delete qimg;
    delete out;
    return ret;
  }

  return QImage();
}


YuvFrame* VP8Decoder::decodeFrameRaw( const QByteArray &data )
{
  // Read frame size from header.
  const uchar *header = extractData( data, 0, 4 );
  size_t frame_sz = mem_get_le32( header );
  delete header;
  vpx_codec_iter_t iter = NULL;
  vpx_image_t *img;

  const uint8_t *frame = extractData( data, IVF_FRAME_HDR_SZ, frame_sz );
  if( !frame ) // Stuff went horribly wrong.
  {
    const QString msg( "Failed to read frame" );
    emit error( msg );
    return NULL;
  }
  
  _frames++;
 
  /* Decode the frame */
  if( vpx_codec_decode( &_codec, frame, frame_sz, NULL, 0 ) )
  {
    const QString msg( "Failed to decode frame: %1 - %2" );
    const char* err = vpx_codec_error( &_codec );
    const char* errdetail = vpx_codec_error_detail( &_codec );
    emit error( msg.arg( err ).arg( errdetail ) );
    return NULL;
  }
 
  /* Convert decoded data */
  YuvFrame *ret = NULL;
  if( (img = vpx_codec_get_frame( &_codec, &iter )) ) { // <-------------------------------- Originally while-loop
    /*switch( img->fmt ) {
    case VPX_IMG_FMT_I420:
      qDebug() << QString( "Image format: VPX_IMG_FMT_I420" );
      break;
    case VPX_IMG_FMT_RGB24:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB24" );
      break;
    case VPX_IMG_FMT_RGB32:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB32" );
      break;
    case VPX_IMG_FMT_RGB565:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB565" );
      break;
    case VPX_IMG_FMT_RGB555:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB555" );
      break;
    case VPX_IMG_FMT_UYVY:
      qDebug() << QString( "Image format: VPX_IMG_FMT_UYVY" );
      break;
    case VPX_IMG_FMT_YUY2:
      qDebug() << QString( "Image format: VPX_IMG_FMT_YUY2" );
      break;
    case VPX_IMG_FMT_YVYU:
      qDebug() << QString( "Image format: VPX_IMG_FMT_YVYU" );
      break;
    case VPX_IMG_FMT_BGR24:
      qDebug() << QString( "Image format: VPX_IMG_FMT_BGR24" );
      break;
    case VPX_IMG_FMT_RGB32_LE:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB32_LE" );
      break;
    case VPX_IMG_FMT_ARGB:
      qDebug() << QString( "Image format: VPX_IMG_FMT_ARGB" );
      break;
    case VPX_IMG_FMT_ARGB_LE:
      qDebug() << QString( "Image format: VPX_IMG_FMT_ARGB_LE" );
      break;
    case VPX_IMG_FMT_RGB565_LE:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB565_LE" );
      break;
    case VPX_IMG_FMT_RGB555_LE:
      qDebug() << QString( "Image format: VPX_IMG_FMT_RGB555_LE" );
      break;
    case VPX_IMG_FMT_YV12:
      qDebug() << QString( "Image format: VPX_IMG_FMT_YV12" );
      break;
    case VPX_IMG_FMT_VPXI420:
      qDebug() << QString( "Image format: VPX_IMG_FMT_VPXI420" );
      break;
    default:
      qDebug() << QString( "Image format: Unknown" );
    }*/

    ret = new YuvFrame;
    ret->width = img->d_w;
    ret->height = img->d_h;
    ret->y = (unsigned char*)malloc( sizeof(unsigned char) * ret->width * ret->height );
    ret->u = (unsigned char*)malloc( sizeof(unsigned char) * (ret->width >> 1) * (ret->height >> 1) );
    ret->v = (unsigned char*)malloc( sizeof(unsigned char) * (ret->width >> 1) * (ret->height >> 1) );
    
    unsigned char *y = ret->y;
    unsigned char *u = ret->u;
    unsigned char *v = ret->v;

    unsigned char *s = img->planes[0];
    for( int l = 0; l < ret->height; l++ ) {
      memcpy( y, s, ret->width );
      y += ret->width;
      s += img->stride[0];
    }

    s = img->planes[1];
    for( int l = 0; l < (ret->height >> 1); l++ ) {
      memcpy( u, s, (ret->width >> 1) );
      u += (ret->width >> 1);
      s += img->stride[1];
    }

    s = img->planes[2];
    for( int l = 0; l < (ret->height >> 1); l++ ) {
      memcpy( v, s, (ret->width >> 1) );
      v += (ret->width >> 1);
      s += img->stride[1];
    }
  }

  if( frame )
    delete frame;

  return ret;
}


void VP8Decoder::finish()
{
  QMutexLocker l( &_mutex );
  _stop = true;

  vpx_codec_destroy( &_codec );
}


void VP8Decoder::stop()
{
  QMutexLocker l( &_mutex );
  _stop = true;
}


void VP8Decoder::resume()
{
  QMutexLocker l( &_mutex );
  _stop = false;
}
