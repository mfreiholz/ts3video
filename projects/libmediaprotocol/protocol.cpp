#if defined(_WIN32)
#include <WinSock2.h>
#elif defined(__linux__)
#include <netinet/in.h>
#define htonll(x) x
#define ntohll(x) x
#endif

#include <math.h>
#include <cstring>

#include "protocol.h"

namespace UDP {

///////////////////////////////////////////////////////////////////////

bool Datagram::write(FILE* f) const
{
	fwrite(&this->magic, sizeof(dg_magic_t), 1, f);
	fwrite(&this->type, sizeof(dg_type_t), 1, f);
	return true;
}

bool Datagram::read(FILE* f)
{
	fread(&this->magic, sizeof(dg_magic_t), 1, f);
	fread(&this->type, sizeof(dg_type_t), 1, f);
	return true;
}

///////////////////////////////////////////////////////////////////////

bool AuthDatagram::write(FILE* f) const
{
	Datagram::write(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fwrite(&this->size, sizeof(dg_size_t), 1, f);
	fwrite(this->data, sizeof(dg_byte_t), this->size, f);
#else
	const dg_size_t nsize = htons(this->size);
	fwrite(&nsize, sizeof(dg_size_t), 1, f);
	fwrite(this->data, sizeof(dg_byte_t), this->size, f);
#endif

	return true;
}

bool AuthDatagram::read(FILE* f)
{
	Datagram::read(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fread(&this->size, sizeof(dg_size_t), 1, f);
	fread(this->data, sizeof(dg_byte_t), this->size, f);
#else
	dg_size_t nsize;
	fread(&nsize, sizeof(dg_byte_t), 1, f);
	this->size = ntohs(nsize);
	fread(this->data, sizeof(dg_byte_t), this->size, f);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////

bool VideoFrameDatagram::write(FILE* f) const
{
	Datagram::write(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fwrite(&this->flags, sizeof(dg_flags_t), 1, f);
	fwrite(&this->sender, sizeof(dg_sender_t), 1, f);
	fwrite(&this->frameId, sizeof(dg_frame_id_t), 1, f);
	fwrite(&this->index, sizeof(dg_data_index_t), 1, f);
	fwrite(&this->count, sizeof(dg_data_count_t), 1, f);
	fwrite(&this->size, sizeof(dg_size_t), 1, f);
	fwrite(this->data, sizeof(dg_byte_t), this->size, f);
#else
	const dg_flags_t nflags = this->flags;
	fwrite(&nflags, sizeof(dg_flags_t), 1, f);
	const dg_sender_t nsender = htons(this->sender);
	fwrite(&nsender, sizeof(dg_sender_t), 1, f);
	const dg_frame_id_t nframeId = htonll(this->frameId);
	fwrite(&nframeId, sizeof(dg_frame_id_t), 1, f);
	const dg_data_index_t nindex = htons(this->index);
	fwrite(&nindex, sizeof(dg_data_index_t), 1, f);
	const dg_data_count_t ncount = htons(this->count);
	fwrite(&ncount, sizeof(dg_data_count_t), 1, f);
	const dg_size_t nsize = htons(this->size);
	fwrite(&nsize, sizeof(dg_size_t), 1, f);
	fwrite(this->data, sizeof(dg_byte_t), this->size, f);
#endif

	return true;
}

bool VideoFrameDatagram::read(FILE* f)
{
	Datagram::read(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fread(&this->flags, sizeof(dg_flags_t), 1, f);
	fread(&this->sender, sizeof(dg_sender_t), 1, f);
	fread(&this->frameId, sizeof(dg_frame_id_t), 1, f);
	fread(&this->index, sizeof(dg_data_index_t), 1, f);
	fread(&this->count, sizeof(dg_data_count_t), 1, f);
	fread(&this->size, sizeof(dg_size_t), 1, f);
	if (!this->data)
	{
		this->data = new dg_byte_t[this->size];
	}
	fread(this->data, sizeof(dg_byte_t), this->size, f);
#else
	dg_flags_t nflags;
	fread(&nflags, sizeof(dg_flags_t), 1, f);
	this->flags = nflags;

	dg_sender_t nsender;
	fread(&nsender, sizeof(dg_sender_t), 1, f);
	this->sender = ntohs(nsender);

	dg_frame_id_t nframeId;
	fread(&nframeId, sizeof(dg_frame_id_t), 1, f);
	this->frameId = ntohll(nframeId);

	dg_data_index_t nindex;
	fread(&nindex, sizeof(dg_data_index_t), 1, f);
	this->index = ntohs(nindex);

	dg_data_count_t ncount;
	fread(&ncount, sizeof(dg_data_count_t), 1, f);
	this->count = ntohs(ncount);

	dg_size_t nsize;
	fread(&nsize, sizeof(dg_size_t), 1, f);
	this->size = ntohs(nsize);

	if (!this->data)
	{
		this->data = new dg_byte_t[this->size];
	}
	fread(this->data, sizeof(dg_byte_t), this->size, f);
#endif

	return true;
}

int VideoFrameDatagram::split(const dg_byte_t* data, size_t dataLength,
							  dg_frame_id_t frameId, dg_sender_t senderId, VideoFrameDatagram** *datagrams_,
							  VideoFrameDatagram::dg_data_count_t& datagramsLength_)
{
	const dg_byte_t* pdata = data;
	if (!data || dataLength <= 0)
	{
		return -1;
	}

	// Calculate and create buffer.
	datagramsLength_ = (dg_data_count_t) ceil((double) dataLength /
					   VideoFrameDatagram::MAXSIZE);
	*datagrams_ = new VideoFrameDatagram*[datagramsLength_];

	// Fill buffer.
	for (dg_data_count_t i = 0; i < datagramsLength_; ++i)
	{
		VideoFrameDatagram* dg = new VideoFrameDatagram();
		dg->flags = VideoFrameDatagram::None;
		dg->sender = senderId;
		dg->frameId = frameId;
		dg->index = i;
		dg->count = datagramsLength_;
		dg->size = 0;
		dg->data = 0;
		(*datagrams_)[i] = dg;

		size_t offset = i * VideoFrameDatagram::MAXSIZE;
		dg_size_t len = VideoFrameDatagram::MAXSIZE;
		if (offset + len > dataLength)
		{
			len = dataLength - offset;
		}
		dg->size = len;
		dg->data = new dg_byte_t[len];
		memcpy(dg->data, pdata, len);
		pdata += len;
	}
	return 0;
}

void VideoFrameDatagram::freeData(VideoFrameDatagram** datagrams,
								  dg_data_count_t length)
{
	for (dg_data_count_t i = 0; i < length; ++i)
	{
		delete datagrams[i];
	}
	delete[] datagrams;
}

///////////////////////////////////////////////////////////////////////

bool VideoFrameRequestRecoveryDatagram::write(FILE* f) const
{
	Datagram::write(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fwrite(&this->sender, sizeof(VideoFrameDatagram::dg_sender_t), 1, f);
	fwrite(&this->frameId, sizeof(VideoFrameDatagram::dg_frame_id_t), 1, f);
	fwrite(&this->index, sizeof(VideoFrameDatagram::dg_data_index_t), 1, f);
#else
	const VideoFrameDatagram::dg_sender_t nsender = htons(this->sender);
	fwrite(&nsender, sizeof(VideoFrameDatagram::dg_sender_t), 1, f);

	const VideoFrameDatagram::dg_frame_id_t nframeId = htonll(this->frameId);
	fwrite(&nframeId, sizeof(VideoFrameDatagram::dg_frame_id_t), 1, f);

	const VideoFrameDatagram::dg_data_index_t nindex = htons(this->index);
	fwrite(&nindex, sizeof(VideoFrameDatagram::dg_data_index_t), 1, f);
#endif

	return true;
}

bool VideoFrameRequestRecoveryDatagram::read(FILE* f)
{
	Datagram::read(f);

#ifdef UDP_USE_HOSTBYTEORDER
	fread(&this->sender, sizeof(VideoFrameDatagram::dg_sender_t), 1, f);
	fread(&this->frameId, sizeof(VideoFrameDatagram::dg_frame_id_t), 1, f);
	fread(&this->index, sizeof(VideoFrameDatagram::dg_data_index_t), 1, f);
#else
	VideoFrameDatagram::dg_sender_t nsender;
	fread(&nsender, sizeof(VideoFrameDatagram::dg_sender_t), 1, f);
	this->sender = ntohs(nsender);

	VideoFrameDatagram::dg_frame_id_t nframeId;
	fread(&nframeId, sizeof(VideoFrameDatagram::dg_frame_id_t), 1, f);
	this->frameId = ntohll(nframeId);

	VideoFrameDatagram::dg_data_index_t nindex;
	fread(&nindex, sizeof(VideoFrameDatagram::dg_data_index_t), 1, f);
	this->index = ntohs(nindex);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////
// Audio
///////////////////////////////////////////////////////////////////////

bool AudioFrameDatagram::write(FILE* f) const
{
	Datagram::write(f);

#ifdef UDP_USE_HOSTBYTEORDER

#else

#endif

	return true;
}

bool AudioFrameDatagram::read(FILE* f)
{
	Datagram::read(f);

#ifdef UDP_USE_HOSTBYTEORDER

#else

#endif

	return true;
}

int AudioFrameDatagram::split(const dg_byte_t* data, size_t dataLength,
							  AudioFrameDatagram::dg_frame_id_t frameId,
							  AudioFrameDatagram::dg_sender_t senderId, AudioFrameDatagram** *datagrams_,
							  AudioFrameDatagram::dg_data_count_t& datagramsLength_)
{
	const dg_byte_t* pdata = data;
	if (!data || dataLength <= 0)
	{
		return -1;
	}

	// Calculate and create buffer.
	datagramsLength_ = (dg_data_count_t)ceil((double)dataLength /
					   AudioFrameDatagram::MAXSIZE);
	*datagrams_ = new AudioFrameDatagram*[datagramsLength_];

	// Fill buffer.
	for (dg_data_count_t i = 0; i < datagramsLength_; ++i)
	{
		AudioFrameDatagram* dg = new AudioFrameDatagram();
		dg->sender = senderId;
		dg->frameId = frameId;
		dg->index = i;
		dg->count = datagramsLength_;
		dg->size = 0;
		dg->data = 0;
		(*datagrams_)[i] = dg;

		size_t offset = i * AudioFrameDatagram::MAXSIZE;
		dg_size_t len = AudioFrameDatagram::MAXSIZE;
		if (offset + len > dataLength)
		{
			len = dataLength - offset;
		}
		dg->size = len;
		dg->data = new dg_byte_t[len];
		memcpy(dg->data, pdata, len);
		pdata += len;
	}
	return 0;
}

void AudioFrameDatagram::freeData(AudioFrameDatagram** datagrams,
								  AudioFrameDatagram::dg_data_count_t length)
{
	for (dg_data_count_t i = 0; i < length; ++i)
	{
		delete datagrams[i];
	}
	delete[] datagrams;
}

} // End of namespace.
