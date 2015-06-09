#ifndef _VP8DECODER_HEADER_
#define _VP8DECODER_HEADER_

#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtGui/QImage>

class YuvFrame;

/*!
  \class VP8Decoder

  This class decodes VP8 video data into RGB frames.
*/
class VP8Decoder
  : public QObject
{
  Q_OBJECT

public:
  VP8Decoder(QObject *parent = 0);

  vpx_codec_ctx_t & getCodec();
  int getFrameCount();

public slots:
  void initialize();
  QImage decodeFrame( const QByteArray &frame );
  YuvFrame* decodeFrameRaw( const QByteArray &frame );
  void finish();
  void stop();
  void resume();

protected:

signals:
  void error( const QString msg );
  void frameDecoded( const QImage &arr );
  void finished( int frameCount );

private:
  vpx_codec_ctx_t _codec;
  int _frames;
  vpx_image_t _raw;
  int _flags;
  unsigned int _width;
  unsigned int _height;
  bool _stop;
  QMutex _mutex;
};

#endif