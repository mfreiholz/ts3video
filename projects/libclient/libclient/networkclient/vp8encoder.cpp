#include "vp8encoder.h"

#include <QDateTime>

#include "libapp/vp8frame.h"

// VPX defines.
#define VPX_CODEC_DISABLE_COMPAT 1
#define vpxinterface (vpx_codec_vp8_cx())
#define fourcc 0x30385056

///////////////////////////////////////////////////////////////////////////////
// Error handling (mfreiholz)
///////////////////////////////////////////////////////////////////////////////

// Recovery flags which should be used, based on the frame type.
unsigned int global_recovery_flags[] =
{
	0,                                              // NORMAL = 0
	VPX_EFLAG_FORCE_KF,                             // KEY = 1
	VP8_EFLAG_FORCE_GF    | VP8_EFLAG_NO_UPD_ARF |
	VP8_EFLAG_NO_REF_LAST | VP8_EFLAG_NO_REF_ARF,   // GOLD = 2
	VP8_EFLAG_FORCE_ARF   | VP8_EFLAG_NO_UPD_GF  |
	VP8_EFLAG_NO_REF_LAST | VP8_EFLAG_NO_REF_GF     // ALTREF = 3
};

///////////////////////////////////////////////////////////////////////////////
// Static helper functions.
///////////////////////////////////////////////////////////////////////////////

static void mem_put_le16(char* mem, unsigned int val)
{
	mem[0] = val;
	mem[1] = val >> 8;
}

static void mem_put_le32(char* mem, unsigned int val)
{
	mem[0] = val;
	mem[1] = val >> 8;
	mem[2] = val >> 16;
	mem[3] = val >> 24;
}

///////////////////////////////////////////////////////////////////////////////
// VP8Encoder
///////////////////////////////////////////////////////////////////////////////

VP8Encoder::VP8Encoder()
	: _codec(),
	  _cfg(),
	  _incremental_frame_number(0),
	  _raw(),
	  _width(0),
	  _height(0),
	  _request_recovery_flag(0)
{
}

VP8Encoder::~VP8Encoder()
{
}

bool VP8Encoder::initialize(int width, int height, int bitrate, int framerate)
{
	_width = width;
	_height = height;

	// Populate encoder configuration.
	vpx_codec_err_t res;
	if ((res = vpx_codec_enc_config_default(vpxinterface, &_cfg, 0)))
	{
		fprintf(stderr, "Failed to get VP8 codec config (error=%s)\n",
				vpx_codec_err_to_string(res));
		return false;
	}

	// Update the default configuration with our settings.
	_cfg.g_w = _width;
	_cfg.g_h = _height;
	_cfg.rc_end_usage = VPX_CBR;
	_cfg.rc_target_bitrate = bitrate;
	_cfg.g_timebase.num = 1;          // Reciproce numerator of framerate.
	_cfg.g_timebase.den = framerate;  // Framerate.
	_cfg.g_error_resilient = 1;
	_cfg.rc_min_quantizer = 4;
	_cfg.rc_max_quantizer = 56;
	_cfg.kf_mode = VPX_KF_DISABLED;  // Further configured with: (VPX_KF_AUTO) _cfg.kf_max_dist = 2000;

	// Initialize codec.
	if ((res = vpx_codec_enc_init(&_codec, vpxinterface, &_cfg, 0)))
	{
		fprintf(stderr, "Failed to initialize VP8 encoder (error=%s)\n",
				vpx_codec_err_to_string(res));
		return false;
	}

	// Initialize raw frame container, which is used
	// later in encoding steps as some kind a buffer.
	vpx_img_alloc(&_raw, VPX_IMG_FMT_YV12, width, height, 1);
	return true;
}

bool VP8Encoder::isValidFrame(const YuvFrame& frame) const
{
	if (_cfg.g_w != frame.width || _cfg.g_h != frame.height)
		return false;
	return true;
}

VP8Frame* VP8Encoder::encode(YuvFrame& yuvFrame)
{
	// Set encoder raw frame.
	_raw.planes[0] = yuvFrame.y;
	_raw.planes[1] = yuvFrame.u;
	_raw.planes[2] = yuvFrame.v;

	VP8Frame* vp8_frame = new VP8Frame();
	vp8_frame->time = ++_incremental_frame_number;  //QDateTime::currentMSecsSinceEpoch();

	// Add possibility for recovery.
	vpx_enc_frame_flags_t flags = global_recovery_flags[_request_recovery_flag];

	// Encode frame.
	vpx_codec_err_t err_flag;
	if ((err_flag = vpx_codec_encode(&_codec, &_raw, vp8_frame->time, 1, flags, VPX_DL_REALTIME)))
	{
		fprintf(stderr, "Can not encode frame (error=%s)\n",
				vpx_codec_err_to_string(err_flag));
		delete vp8_frame;
		return nullptr;
	}

	vpx_codec_iter_t iter = NULL;
	const vpx_codec_cx_pkt_t* pkt;
	QByteArray arr;

	while ((pkt = vpx_codec_get_cx_data(&_codec, &iter)))
	{
		if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
		{

			vp8_frame->type = _request_recovery_flag;
			if (_request_recovery_flag != VP8Frame::NORMAL)
			{
				_request_recovery_flag = VP8Frame::NORMAL;
			}

			const char* buf = (const char*)pkt->data.frame.buf;

			// Create header.
			char header[12];
			vpx_codec_pts_t pts = pkt->data.frame.pts;
			mem_put_le32(header, pkt->data.frame.sz);
			mem_put_le32(header + 4, pts & 0xFFFFFFFF);
			mem_put_le32(header + 8, pts >> 32);

			// Write header and data to array.
			arr.append(header, 12);
			arr.append(buf, pkt->data.frame.sz);
		}
	}
	vp8_frame->data = arr;
	return vp8_frame;
}

void VP8Encoder::setRequestRecoveryFlag(int recoveryFlag)
{
	_request_recovery_flag = recoveryFlag;
}


/*  QByteArray VP8Encoder::finish()
    {
    QMutexLocker l( &_mutex );

    // Reset encoder flag.
    _flags = 0;

    if( vpx_codec_encode( &_codec, NULL, _incremental_frame_number, 1, _flags, VPX_DL_REALTIME ) )
    {
    const QString msg( "Failed to encode final frame" );
    emit error( msg );
    return NULL;
    }

    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt;
    QByteArray arr;

    while( ( pkt = vpx_codec_get_cx_data( &_codec, &iter ) ) ) {
    if( pkt->kind == VPX_CODEC_CX_FRAME_PKT ) {
      const char *buf = (const char*)pkt->data.frame.buf;

      // Create header.
      char header[12];
      vpx_codec_pts_t pts = pkt->data.frame.pts;
      mem_put_le32( header, pkt->data.frame.sz );
      mem_put_le32( header+4, pts&0xFFFFFFFF );
      mem_put_le32( header+8, pts >> 32 );

      // Write header and data to array.
      arr.append( header, 12 );
      arr.append( buf, pkt->data.frame.sz );
    }
    }

    ++_incremental_frame_number;

    vpx_codec_destroy(&_codec);
    vpx_img_free( &_raw );

    if( !arr.isNull() )
    emit frameEncoded( arr );
    emit finished( _incremental_frame_number );
    return arr;
    }*/


/*****************************************************************************/
/* VideoWriter                                                               */
/*****************************************************************************/
/*

    VideoWriter::VideoWriter( QObject *parent )
    : QObject( parent )
    , _file( 0 )
    , _framesWritten( 0 )
    {
    }


    VideoWriter::~VideoWriter()
    {
    if( _file )
    {
    if( _file->isOpen() )
      _file->close();
    delete _file;
    }
    }


    void VideoWriter::waitForFinished()
    {
    QMutexLocker l( &_mutex );
    return;
    }


    bool VideoWriter::initialize( const QString &filepath, bool overwrite, vpx_codec_enc_cfg_t *cfg )
    {
    QMutexLocker l( &_mutex );

    // Close opened files.
    if( _file )
    delete _file;
    // Set new file path.
    _file = new QFile( filepath );

    // Does file already exist?
    if( _file->exists() )
    {
    if( overwrite ) {
      _file->remove();
    } else {
      const QString msg( "File already exists" );
      emit error( msg );
      delete _file;
      _file = NULL;
      return false;
    }
    }

    // Open file.
    if( !_file->open( QIODevice::WriteOnly ) )
    {
    const QString msg( "Opening file for writing failed" );
    emit error( msg );
    delete _file;
    _file = NULL;
    return false;
    }

    // Reset written frames counter.
    _framesWritten = 0;

    // Prepare file header.
    char header[32];
    if( cfg )
    {
    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16( header+4,  0 );                   // version
    mem_put_le16( header+6,  32 );                  // headersize
    mem_put_le32( header+8,  fourcc );              // headersize
    mem_put_le16( header+12, cfg->g_w );            // width
    mem_put_le16( header+14, cfg->g_h );            // height
    mem_put_le32( header+16, cfg->g_timebase.den ); // rate
    mem_put_le32( header+20, cfg->g_timebase.num ); // scale
    mem_put_le32( header+24, 0 );                   // length - zero for now
    mem_put_le32( header+28, 0 );                   // unused
    }
    _file->write( header, 32 );


    return true;
    }


    void VideoWriter::writeFrame( const QByteArray &data )
    {
    QMutexLocker l( &_mutex );

    // Header already included.
    // Write directly to file.
    _file->write( data );
    _framesWritten++;
    }


    void VideoWriter::finish( vpx_codec_enc_cfg_t *cfg, int frames )
    {
    QMutexLocker l( &_mutex );

    // Write file header.
    char header[32];

    if(cfg->g_pass != VPX_RC_ONE_PASS && cfg->g_pass != VPX_RC_LAST_PASS)
    return;

    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header+4,  0);                   // version
    mem_put_le16(header+6,  32);                  // headersize
    mem_put_le32(header+8,  fourcc);              // headersize
    mem_put_le16(header+12, cfg->g_w);            // width
    mem_put_le16(header+14, cfg->g_h);            // height
    mem_put_le32(header+16, cfg->g_timebase.den); // rate
    mem_put_le32(header+20, cfg->g_timebase.num); // scale
    mem_put_le32(header+24, frames);              // length
    mem_put_le32(header+28, 0);                   // unused

    // Return to beginning of file.
    if( !_file->seek( 0 ) )
    {
    const QString msg( "Couldn't write file header" );
    emit error( msg );
    _file->close();
    delete _file;
    _file = NULL;
    return;
    }

    _file->write( header, 32 );

    emit finished();
    }
*/

/*****************************************************************************/
/* RecordingThread                                                               */
/*****************************************************************************/

/*
    RecordingThread::RecordingThread( QObject *parent )
    : QThread(parent)
    , _buffer( NULL )
    , _bufferMutex()
    , _cond()
    , _stopCond()
    , _stop( true )
    , _finish( false )
    , _encoder()
    , _writer()
    {

    }


    RecordingThread::~RecordingThread()
    {
    if( _buffer )
    delete _buffer;
    }


    void RecordingThread::initialize( const QString &filepath, int width, int height, int bitrate, bool overwrite )
    {
    _encoder.initialize( width, height, bitrate );
    _writer.initialize( filepath, overwrite, &_encoder.getConfig() );

    QObject::connect( &_encoder, SIGNAL( error( const QString ) ), this, SIGNAL( error( const QString ) ) );
    QObject::connect( &_writer, SIGNAL( error( const QString ) ), this, SIGNAL( error( const QString ) ) );
    }


    void RecordingThread::setNextFrame( const QImage &frame )
    {
    QMutexLocker bufl( &_bufferMutex );
    if( _buffer )
    delete _buffer;
    _buffer = new QImage( frame );
    _cond.wakeAll();
    }


    void RecordingThread::finish()
    {
    QMutexLocker stopl( &_stopMutex );
    _stop = true;
    _finish = true;

    // If the encoder is waiting for new input, wake it to finish.
    _cond.wakeAll();
    }


    void RecordingThread::stop()
    {
    QMutexLocker stopl( &_stopMutex );
    _stop = true;

    // If the encoder is waiting for new input, wake it to stop.
    _cond.wakeAll();
    }


    void RecordingThread::run()
    {
    QMutexLocker stopl( &_stopMutex );
    _stop = false;
    _finish = false;

    while( !_stop )
    {
    // Unlock stop mutex.
    stopl.unlock();

    // Create safe copy from buffer.
    QMutexLocker bufl( &_bufferMutex );
    if( !_buffer ) // Wait if no new frame is available.
      _cond.wait( &_bufferMutex );
    if( !_buffer ) { // In case there is still no frame, restart loop.
      bufl.unlock();
      stopl.relock();
      continue;
    }
    QImage frame = _buffer->copy(); // Deep copy.
    delete _buffer;
    _buffer = NULL;
    bufl.unlock();

    // Encode frame.
    QByteArray data;// = _encoder.encodeFrame( frame );
    // Write data.
    if( !data.isNull() ) {
      _writer.writeFrame( data );
    }

    // Lock stop mutex.
    stopl.relock();
    }

    if( _finish ) {
    // Encode last frame.
    QByteArray data = _encoder.finish();
    // Write data.
    if( !data.isNull() ) {
      _writer.writeFrame( data );
    }

    // Finish file output.
    vpx_codec_enc_cfg_t *cfg = &_encoder.getConfig();
    int frames = _encoder.getFrameCount() - 1;
    _writer.finish( cfg, frames );
    }

    _stopCond.wakeAll();
    }
*/
