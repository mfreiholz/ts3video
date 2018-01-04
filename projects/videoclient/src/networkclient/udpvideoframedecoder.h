#pragma once

#include <vector>
#include <queue>
#include <map>

#include <QByteArray>

#include "libmediaprotocol/protocol.h"

class VP8Frame;

typedef std::vector<UDP::VideoFrameDatagram*> DGPtrList;
typedef std::map<unsigned long long, struct DGPtrListMapValue*> DGPtrListMap;


struct DGPtrListMapValue
{
	// Timestamp when the the first datagram in <em>datagrams</em>
	// has been added.
	unsigned long long first_received_datagram_time;
	DGPtrList datagrams;
};


/*!
	This class provides functionality to collect UdpVideoFrameDatagram objects
	and merge them into VideoFrame objects.

	\see VideoFrameUdpEncoder to create UdpVideoFrameDatagram(s) from VideoFrame(s).
*/
class VideoFrameUdpDecoder
{
public:
	enum ErrorTypes
	{
		NoError,
		InvalidParameter,
		AlreadyProcessed
	};

	/*!
		Initialzes the decoder with values for decoding.

		\param[in,optional] maximum_distinct_frames
		It defines the number of how many distinct frames should be kept in the
		internal buffer for the calculation of the next frame. It makes it possible
		that a video frame with higher id can be added to before another one but the
		lower id returns before the higher id.
	*/
	VideoFrameUdpDecoder();
	~VideoFrameUdpDecoder();

	/*!
		Adds another datagram to the internal buffer <em>_frame_buffers</em> of datagrams.
		As soon as all parts available to create a <em>VideoFrame</em> object it is
		will be removed from the <em>_frame_buffers</em> and is moved to the
		<em>_finished_frames_queue</em>.

		\param[in] dpart
		The datagram which should be added to the internal buffer. The decoder
		takes complete ownership of the datagram. It's also possible that it will
		be deleted after calling this function.

		\return NO_ERROR on success, otherwise ERROR_*.
	*/
	int add(UDP::VideoFrameDatagram* dpart);

	/*!
		Trys to get the next completed <em>VP8Frame</em> object from internal
		<em>_complete_frames_queue</em> queue.

		\note The decoder releases the ownership of the object to the caller.

		\return An VP8Frame object or NULL if no frame is available.
	*/
	VP8Frame* next();
	int getWaitsForType() const;

protected:
	/*!
		Checks whether the given frame buffer is complete and contains all data,
		which is required to create a <em>VideoFrame</em> object from it.

		\param[in] buffer
		Container with datagrams.

		\return true/false
	*/
	bool isComplete(const DGPtrList& buffer);

	/*!
		Creates a <em>VP8Frame</em> object from completed buffer.

		\pre-condition isComplete(buffer)

		\param[in] buffer
		The buffer which contains all required parts.

		\return A new VP8Frame object.
	*/
	VP8Frame* createFrame(const DGPtrList& buffer) const;

	/*!
		Checks whether the decoder has too many frame buffers and deletes the
		oldest buffers as long as the number of buffers is greater than
		<em>_maximum_distinct_frames</em>.
	*/
	void checkFrameBuffers(unsigned int max_size);

	void checkCompleteFramesQueue(unsigned int max_size);

	void removeFromFrameBuffer(unsigned long long ts);

private:
	// Error handling.
	unsigned int _last_error;

	// Frame buffers.
	unsigned int _maximum_distinct_frames;
	DGPtrListMap _frame_buffers;

	// Queue of completely received VP8Frames, sorted by it's ID/Timstamp.
	std::map<unsigned long long, VP8Frame*> _complete_frames_queue;
	unsigned long long _last_completed_frame_id;

	/*
		Holds the number of key-frames, which has been passed through this decoder.
		As long as no key-frame has been received, the next() method will return NULL and
		getWaitsForType() will return VP8Frame::KEY.
	*/
	unsigned long long _received_key_frame_count;

	int _wait_for_frame_type;
	int _wait_timestamp;
};