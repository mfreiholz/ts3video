#include "MediaSocketVideoFrameDecoder.h"
#include "videolib/timeutil.h"

// SingleFrameBuffer //////////////////////////////////////////////////

SingleFrameBuffer::SingleFrameBuffer(size_t capacity) :
	_datagrams(capacity), _datagramsCount(0), firstReceivedTimestamp(0)
{}

SingleFrameBuffer::~SingleFrameBuffer()
{
	for (size_t i = 0; i < _datagrams.size(); ++i)
	{
		delete _datagrams[i];
	}
	_datagrams.clear();
}

bool
SingleFrameBuffer::add(UDP::VideoFrameDatagram* datagram)
{
	if (_datagrams[datagram->index] == NULL)
	{
		_datagrams[datagram->index] = datagram;
		_datagramsCount++;
	}
	else
	{
		delete datagram;
		datagram = NULL;
	}
	return _datagrams.size() == _datagramsCount;
}

bool
SingleFrameBuffer::isComplete() const
{
	return _datagrams.size() == _datagramsCount;
}

// MediaSocketVideoFrameDecoder ///////////////////////////////////////

MediaSocketVideoFrameDecoder::MediaSocketVideoFrameDecoder(MediaSocket& socket) :
	_socket(socket)
{}

MediaSocketVideoFrameDecoder::~MediaSocketVideoFrameDecoder()
{}

void
MediaSocketVideoFrameDecoder::handle(UDP::VideoFrameDatagram* datagram)
{
	SingleFrameBuffer* buffer = NULL;

	// Get existing frame buffer or create one.
	auto itr = _frameBuffers.find(datagram->frameId);
	if (itr == _frameBuffers.end())
	{
		buffer = new SingleFrameBuffer(datagram->count);
		buffer->firstReceivedTimestamp = get_local_timestamp();
		_frameBuffers[datagram->frameId] = buffer;
	}

	// Add to buffer.
	if (buffer->add(datagram))
	{
		// Frame is complete.

	}
	else
	{
		// Frame is not complete.
	}

	// Check if frame is next we wanted.
}
