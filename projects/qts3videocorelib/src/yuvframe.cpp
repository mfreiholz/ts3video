#include "yuvframe.h"

//#include <stdlib.h>
//#include <cstring>
#include <stdint.h>

#include <QtMath>
#include <QImage>

YuvFrame::YuvFrame() :
  width(0),
  height(0),
  y(nullptr),
  u(nullptr),
  v(nullptr)
{}

YuvFrame::~YuvFrame()
{
  if (y) free(y);
  if (u) free(u);
  if (v) free(v);
}

YuvFrame* YuvFrame::copy() const
{
  auto ret = new YuvFrame;
  ret->width = width;
  ret->height = height;
  if (y) {
    ret->y = (unsigned char*) malloc(sizeof(unsigned char) * ret->width * ret->height);
    memcpy(ret->y, y, ret->width * ret->height);
  }
  if (u) {
    ret->u = (unsigned char*) malloc(sizeof(unsigned char) * (ret->width >> 1) * (ret->height >> 1));
    memcpy(ret->u, u, (ret->width >> 1) * (ret->height >> 1));
  }
  if (v) {
    ret->v = (unsigned char*) malloc(sizeof(unsigned char) * (ret->width >> 1) * (ret->height >> 1));
    memcpy(ret->v, v, (ret->width >> 1) * (ret->height >> 1));
  }
  return ret;
}

void YuvFrame::overlayDarkEdge(int posx, int posy, int width, int height)
{
  int maxdist = 30;

  for( int y = 0; y < height; y++ ) {
    for( int x = 0; x < width; x++ ) {

      int distleft = x;
      int distright = width - x;
      int distx;
      if( distleft < distright )
        distx = distleft;
      else
        distx = distright;

      int disttop = y;
      int distbot = height - y;
      int disty;
      if( disttop < distbot )
        disty = disttop;
      else
        disty = distbot;
      
      if( distx > maxdist && disty > maxdist )
        continue;

      if( distx > maxdist )
        distx = maxdist;
      if( disty > maxdist )
        disty = maxdist;
      
      qreal factorX, factorY;
      qreal exp = 1.0 / 3.0;

      factorX = (qreal)distx / (qreal)maxdist;
      factorX = qPow( factorX, exp );
      
      factorY = (qreal)disty / (qreal)maxdist;
      factorY = qPow( factorY, exp );

      float factor = factorX * factorY;
      if( factor >= 1.0f )
        continue;
      if( factor < 0.0f )
        factor = 0.0f;

      int pos = (posy + y) * this->width + (posx + x);
      this->y[pos] *= factor;// * 0.7f + 0.3f;
    }
  }
}

QImage YuvFrame::toQImage() const
{
  uint8_t *rgb = (unsigned char*)malloc(3 * width * height);
  uint8_t *out = rgb;
    
  // First plane from codec is 1:1 Y (luminance) data.
  uint8_t *py = y;
    
  // Second and third planes are 1:2 subsampled U/V.
  uint8_t *tpu = (uint8_t*)malloc( width * height );
  uint8_t *pu = tpu;
  uint8_t *tpv = (uint8_t*)malloc( width * height );
  uint8_t *pv = tpv;
    
  // High quality 1:2 upsampling of U and V using Lanczos filter.
  lanczos_interp2( u, pu, width >> 1, width, height );
  lanczos_interp2( v, pv, width >> 1, width, height );
    
  for( uint j = 0; j < height; ++j ) {
    for( uint i = 0; i < width; ++i ) {
      int y2 = py[i] - 16;
      int u2 = pu[i] - 128;
      int v2 = pv[i] - 128;
      rgb[0] = clamp( SCALEYUV( rcoeff( y2, u2, v2 ) ) );
      rgb[1] = clamp( SCALEYUV( gcoeff( y2, u2, v2 ) ) );
      rgb[2] = clamp( SCALEYUV( bcoeff( y2, u2, v2 ) ) );
      rgb += 3;
    }
    py += width;
    pu += width;
    pv += width;
  }

  // Clean-up.
  free(tpu);
  free(tpv);

  QImage image(out, width, height, QImage::Format_RGB888);
  image = image.copy();
  free(out);
  return image;
}

YuvFrame* YuvFrame::fromQImage(const QImage& img)
{
  size_t numpixel = img.width() * img.height();
  int bytesPerPixel = img.depth() >> 3;

  // Get raw image data.
  const unsigned char *constorig = img.bits();
  unsigned char *orig = (unsigned char*)malloc( bytesPerPixel * sizeof( char ) * numpixel );
  memcpy( orig, constorig, bytesPerPixel * sizeof( char ) * numpixel );

  // Allocate memory for YV12 conversion.
  size_t ysize = sizeof( unsigned char ) * img.width() * img.height();
  size_t uvsize = (size_t)(sizeof( unsigned char ) * (img.width() >> 1) * (img.height() >> 1));

  unsigned char *ydata = (unsigned char*)malloc( ysize );
  unsigned char *udata = (unsigned char*)malloc( uvsize );
  unsigned char *vdata = (unsigned char*)malloc( uvsize );

  rgbToYV12( orig, img.width(), img.height(), ydata, udata, vdata, imageFormat( img.format() ) );

  YuvFrame *yuv = new YuvFrame();
  yuv->width = img.width();
  yuv->height = img.height();
  yuv->y = ydata;
  yuv->u = udata;
  yuv->v = vdata;

  free( orig );
  return yuv;
}

YuvFrame* YuvFrame::fromRgb( const unsigned char *rgb, uint width, uint height, const ImageFormat &imgFormat, uint stride, bool bottomUp )
{
  uint numpixel = width * height;
  if( stride == 0 )
    stride = width;

  int bytesPerPixel;
  switch( imgFormat ) { // RGB24, ARGB32, BGA24, BGRA32
  case RGB24:
  case BGA24:
    bytesPerPixel = 3;
    break;
  case ARGB32:
  case BGRA32:
    bytesPerPixel = 4;
    break;
  }

  int bytesPerLine = bytesPerPixel * width;

  // Get raw image data.
  const unsigned char *in = nullptr;
  if( bottomUp )
    in = rgb + (stride * (height - 1));
  else
    in = rgb;

  unsigned char *orig = (unsigned char*)malloc( bytesPerPixel * numpixel );
  unsigned char *out = orig;
  for( int i = 0; i < height; i++ ) {
    memcpy( out, in, bytesPerLine );
    out += bytesPerLine;

    if( bottomUp )
      in -= stride;
    else
      in += stride;
  }

  // Allocate memory for YV12 conversion.
  uint ysize = sizeof( unsigned char ) * numpixel;
  uint uvsize = ysize >> 2;

  unsigned char *ydata = (unsigned char*)malloc( ysize );
  unsigned char *udata = (unsigned char*)malloc( uvsize );
  unsigned char *vdata = (unsigned char*)malloc( uvsize );

  rgbToYV12( orig, width, height, ydata, udata, vdata, imgFormat );

  YuvFrame *yuv = new YuvFrame();
  yuv->width = width;
  yuv->height = height;
  yuv->y = ydata;
  yuv->u = udata;
  yuv->v = vdata;

  free( orig );
  return yuv;
}

YuvFrame* YuvFrame::createBlackImage(uint width, uint height)
{
  // Create YUV frame and initialize parameters.
  YuvFrame *yuv = new YuvFrame();
  yuv->width = width;
  yuv->height = height;

  // Calculate allocation sizes...
  size_t ysize = sizeof( unsigned char ) * width * height;
  size_t uvsize = (size_t)(sizeof( unsigned char ) * (width >> 1) * (height >> 1));
  // ... and allocate YUV planes.
  yuv->y = (unsigned char*)malloc( ysize );
  yuv->u = (unsigned char*)malloc( uvsize );
  yuv->v = (unsigned char*)malloc( uvsize );

  // Write Y data.
  unsigned char *ydata = yuv->y;
  for( uint n = 0; n < ysize; n++ ) {
    *ydata = 0x10;
    ydata++;
  }
  
  // Write U and V data.
  unsigned char *udata = yuv->u;
  unsigned char *vdata = yuv->v;
  for( uint n = 0; n < uvsize; n++ ) {
    *udata = 0x80;
    *vdata = 0x80;
    udata++;
    vdata++;
  }
  return yuv;
}

YuvFrame* YuvFrame::create(int width, int height)
{
  // Create YUV frame and initialize parameters.
  YuvFrame *yuv = new YuvFrame();
  yuv->width = width;
  yuv->height = height;

  yuv->y = (unsigned char*)malloc( sizeof(unsigned char) * width * height );
  yuv->u = (unsigned char*)malloc( sizeof(unsigned char) * (width >> 1) * (height >> 1) );
  yuv->v = (unsigned char*)malloc( sizeof(unsigned char) * (width >> 1) * (height >> 1) );
  
  // Return frame.
  return yuv;
}
