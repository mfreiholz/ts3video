#pragma once
#include <vector>
#include <map>
#include "libmediaprotocol/protocol.h"
class MediaSocket;

/*
	Collects a single video-frame from multiple small datagrams.
*/
class SingleFrameBuffer
{
public:
	SingleFrameBuffer(size_t capacity);
	~SingleFrameBuffer();
	bool add(UDP::VideoFrameDatagram* datagram);
	bool isComplete() const;

private:
	std::vector<UDP::VideoFrameDatagram*> _datagrams;
	size_t _datagramsCount;

public:
	uint64_t firstReceivedTimestamp;
};

/*
*/
class MediaSocketVideoFrameDecoder
{
public:
	MediaSocketVideoFrameDecoder(MediaSocket& socket);
	~MediaSocketVideoFrameDecoder();
	void handle(UDP::VideoFrameDatagram* dg);

private:
	MediaSocket& _socket;
	std::map<UDP::VideoFrameDatagram::dg_frame_id_t, SingleFrameBuffer*> _frameBuffers;
};
