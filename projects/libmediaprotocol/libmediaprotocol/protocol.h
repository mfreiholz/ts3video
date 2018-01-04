#ifndef UDPPROTOCOL_HEADER
#define UDPPROTOCOL_HEADER

#include <stdint.h>
#include <cstdio>

#include "libbase/defines.h"

// Declares whether the write/read methods of Datagrams
// should use Little- or Big-Endian byte order.
// If not defined, the methods will use network byte order (BE).
// Windows = LE, Mac = BE, Network = BE
//#define UDP_USE_HOSTBYTEORDER

namespace UDP {

typedef uint16_t dg_size_t;
typedef uint8_t dg_byte_t;

/*!
    The maximum possible size of the datagram content is 65536 (uint16_t = 2^16 = 65536).
    Datagrams of 512 bytes should pass every router on the internet.
*/
class Datagram
{
public:
	typedef uint8_t dg_magic_t;
	typedef uint8_t dg_type_t; // Range: 0x00-0xFF (0-255)

	static const dg_magic_t MAGIC = 0xAE;
	static const dg_size_t MAXSIZE = 512 - sizeof(dg_type_t) - sizeof(dg_magic_t);

	Datagram() : magic(MAGIC), type(0) {}
	Datagram(dg_type_t t) : magic(MAGIC), type(t) {}
	bool write(FILE* f) const;
	bool read(FILE* f);

	dg_magic_t magic;
	dg_type_t type;
};

/*!
*/
class AuthDatagram : public Datagram
{
public:
	static const dg_type_t TYPE = 0x01;
	static const dg_size_t MAXSIZE = Datagram::MAXSIZE - sizeof(dg_size_t);

	AuthDatagram() : Datagram(TYPE), size(0), data(0) {}
	~AuthDatagram()
	{
		delete[] data;
	}
	bool write(FILE* f) const;
	bool read(FILE* f);

	dg_size_t size;
	dg_byte_t* data;
};

/*
*/
class KeepAliveDatagram : public Datagram
{
public:
	static const dg_type_t TYPE = 0x10;
	static const dg_size_t MAXSIZE = Datagram::MAXSIZE - 0;

	KeepAliveDatagram() : Datagram(TYPE) {}
	~KeepAliveDatagram() {}
};

///////////////////////////////////////////////////////////////////////
// Video
///////////////////////////////////////////////////////////////////////

/*!
    Used to send a video-frame over network with the UDP protocol.

    Whats the maximum possible frame size?
    ======================================
    Basically it highly depends on the Datagram::MAXSIZE.
    UDP-Datagrams with a size of 512 byte should go through every router on the internet (by spec).

    Example calculation with a max UDP-Datagram size of 512 bytes
    -------------------------------------------------------------
    First we need to find out, how much bytes we have left in the VideoFrameDatagram for real frame data.

      <leftBytes> = <MaxDatagramSize> - <DatagramHeaderSize>
      <leftBytes> = <leftBytes> - <VideoFrameDatagramHeaderSize>

      leftBytes = 512 bytes -  2 bytes = 510 bytes
      leftBytes = 510 bytes - 17 bytes = 493 bytes
      leftBytes = 493 bytes

    Next, we need to find out how many VideoFrameDatagram objects can be used for a single frame.
    This is based on the actual data-type of the dg_data_index_t and dg_data_count_t (They always must have the same type!).

      <MaxVFDatagrams> = 2^sizeof(dg_data_count_t)

      MaxVFDatagrams = 2^16 = 65536

    Now its possible to calculate the maximum size of a single frame.

      <MaxFrameSize> = <MaxVFDatagrams> * <leftBytes>

      MaxFrameSize = 65536 * 493 bytes = 32309248 bytes = 31552 KB = 30,8125 MB

    Thats it. It's possible to transfer single-frames with a size up to ~30 MB

*/
class VideoFrameDatagram : public Datagram
{
public:
	typedef uint8_t dg_flags_t;
	typedef ocs::clientid_t dg_sender_t;
	typedef uint64_t dg_frame_id_t;
	typedef uint16_t dg_data_index_t;
	typedef uint16_t dg_data_count_t;

	const static dg_type_t TYPE = 0x02;
	const static dg_size_t MAXSIZE = Datagram::MAXSIZE - (sizeof(
										 dg_flags_t) + sizeof(dg_sender_t) + sizeof(dg_frame_id_t) + sizeof(
										 dg_data_index_t) + sizeof(dg_data_count_t) + sizeof(dg_size_t));

	enum Flags { None = 0, Encrypted = 1, Redundant = 2, Flag3 = 4, Flag4 = 8, Flag5 = 16, Flag6 = 32, Flag7 = 64, Flag8 = 128 };

	VideoFrameDatagram() : Datagram(TYPE), flags(0), sender(0), frameId(0),
		index(0), count(0), size(0), data(0) {}
	~VideoFrameDatagram()
	{
		delete[] data;
	}
	bool write(FILE* f) const;
	bool read(FILE* f);
	static int split(const dg_byte_t* data, size_t dataLength,
					 dg_frame_id_t frameId, dg_sender_t senderId, VideoFrameDatagram** *datagrams_,
					 VideoFrameDatagram::dg_data_count_t& datagramsLength_);
	static void freeData(VideoFrameDatagram** datagrams, dg_data_count_t length);

	dg_flags_t flags; ///< Custom flags for the frame.
	dg_sender_t sender; ///< ID of the sender. The server will override this value.
	dg_frame_id_t
	frameId; ///< Used to associated multiple datagrams to a single frame. It is common to use the timestamp of the frame.
	dg_data_index_t index; ///< Part-index of the entire frame.
	dg_data_count_t count; ///< Number of parts for the entire frame.
	dg_size_t size; ///< Size of the "data".
	dg_byte_t* data; ///< Raw frame bytes.
};

/*!
	Send from client to request another client for a resend of
	a part/complete video frame.
*/
class VideoFrameRequestRecoveryDatagram : public Datagram
{
public:
	const static dg_type_t TYPE = 0x03;
	const static dg_size_t MAXSIZE = Datagram::MAXSIZE - (sizeof(
										 VideoFrameDatagram::dg_sender_t) + sizeof(VideoFrameDatagram::dg_frame_id_t) +
									 sizeof(VideoFrameDatagram::dg_data_index_t));

	VideoFrameRequestRecoveryDatagram() :
		Datagram(TYPE),
		sender(0),
		frameId(0),
		index(0)
	{}

	bool write(FILE* f) const;
	bool read(FILE* f);

	VideoFrameDatagram::dg_sender_t
	sender; ///< ID of the source sender of the frame.
	VideoFrameDatagram::dg_frame_id_t
	frameId; ///< If greater than 0 and "index==0", the complete frame will be resend.
	VideoFrameDatagram::dg_data_index_t
	index; ///< If greater than 0, only a single datagram of the frame will be resend.
};

///////////////////////////////////////////////////////////////////////
// Audio
///////////////////////////////////////////////////////////////////////

/*!
*/
class AudioFrameDatagram : public Datagram
{
public:
	typedef ocs::clientid_t dg_sender_t;
	typedef uint64_t dg_frame_id_t;
	typedef uint16_t dg_data_index_t;
	typedef uint16_t dg_data_count_t;

	const static dg_type_t TYPE = 0xA0;
	const static dg_size_t MAXSIZE = Datagram::MAXSIZE - (sizeof(
										 dg_sender_t) + sizeof(dg_frame_id_t) + sizeof(dg_data_index_t) + sizeof(
										 dg_data_count_t) + sizeof(dg_size_t));

	AudioFrameDatagram() : Datagram(TYPE) {}
	bool write(FILE* f) const;
	bool read(FILE* f);

	static int split(const dg_byte_t* data, size_t dataLength,
					 dg_frame_id_t frameId, dg_sender_t senderId, AudioFrameDatagram** *datagrams_,
					 AudioFrameDatagram::dg_data_count_t& datagramsLength_);
	static void freeData(AudioFrameDatagram** datagrams, dg_data_count_t length);

	dg_sender_t sender;
	dg_frame_id_t frameId;
	dg_data_index_t index;
	dg_data_count_t count;
	dg_size_t size;
	dg_byte_t* data;
};

} // End of namespace.
#endif