#include "audioudpdecoder.h"
#include "humblelogging/api.h"
#include <QDataStream>

HUMBLE_LOGGER(HL, "network.udp");

AudioUdpDecoder::AudioUdpDecoder() :
	_lastError(AudioUdpDecoder::NoError),
	_lastCompletedFrameId(0)
{
}

AudioUdpDecoder::~AudioUdpDecoder()
{
}

int AudioUdpDecoder::add(UDP::AudioFrameDatagram* datagram)
{
	if (!datagram)
	{
		_lastError = AudioUdpDecoder::InvalidParameterError;
		return _lastError;
	}

	if (datagram->frameId <= _lastCompletedFrameId)
	{
		delete datagram;
		datagram = NULL;
		_lastError = AudioUdpDecoder::AlreadyProcessedFrameError;
		return _lastError;
	}

	// Get frame buffer
	auto foundBufferItr = _frameBuffers.find(datagram->frameId);
	if (foundBufferItr == _frameBuffers.end())
	{
		_frameBuffers[datagram->frameId] = new FrameBufferValue();
		foundBufferItr = _frameBuffers.find(datagram->frameId);
		(*foundBufferItr).second->datagrams = std::vector<UDP::AudioFrameDatagram*>(datagram->count);
		//(*foundBufferItr).second->firstReceivedDatagramTimestamp = get_local_timestamp();
	}
	auto& buffer = (*foundBufferItr).second->datagrams;
	if (buffer[datagram->index] != NULL)
	{
		delete datagram;
		datagram = nullptr;
		_lastError = AudioUdpDecoder::AlreadyProcessedFrameError;
		return _lastError;
	}
	buffer[datagram->index] = datagram;

	if (!isComplete(buffer))
	{
		checkFrameBuffers();
		_lastError = AudioUdpDecoder::NoError;
		return _lastError;
	}

	// Create VideoFrame object from buffer.
	auto frame = createFrame(buffer);
	_completedFramesQueue[frame->time] = frame;

	checkFrameBuffers();
	checkCompletedFramesQueue();

	_lastError = AudioUdpDecoder::NoError;
	return _lastError;
}

OpusFrame* AudioUdpDecoder::next()
{
	auto begin = _completedFramesQueue.begin();
	if (begin == _completedFramesQueue.end())
	{
		return NULL;
	}
	auto frameId = (*begin).first;
	auto frame = (*begin).second;

	if (frameId != (_lastCompletedFrameId + 1))
	{
		HL_TRACE(HL, QString("Lost frames (current finished frame id=%1; expected frame id=%2)")
				 .arg(frameId).arg(_lastCompletedFrameId + 1).toStdString());
	}
	_completedFramesQueue.erase(begin);
	_lastCompletedFrameId = frameId;
	return frame;
}

bool AudioUdpDecoder::isComplete(const std::vector<UDP::AudioFrameDatagram*>& frameBuffer) const
{
	for (auto i = frameBuffer.begin(), end = frameBuffer.end(); i != end; ++i)
	{
		if (!(*i))
		{
			return false;
		}
	}
	return true;
}

OpusFrame* AudioUdpDecoder::createFrame(const std::vector<UDP::AudioFrameDatagram*>& frameBuffer) const
{
	QByteArray data;
	for (auto i = frameBuffer.begin(), end = frameBuffer.end(); i != end; ++i)
	{
		const auto dg = *i;
		data.append(QByteArray((const char*)dg->data, dg->size));
	}

	// Deserialize QByteArray to VP8Frame.
	auto frame = new OpusFrame();
	QDataStream in(data);
	in >> *frame;
	return frame;
}

void AudioUdpDecoder::checkFrameBuffers(unsigned int maxSize)
{
	while (_frameBuffers.size() > maxSize)
	{
		auto val = (*_frameBuffers.begin()).second;
		auto& fb = (*_frameBuffers.begin()).second->datagrams;
		while (!fb.empty())
		{
			auto dg = fb.back();
			if (dg)
				delete dg;
			fb.pop_back();
		}
		delete val;
		_frameBuffers.erase(_frameBuffers.begin());
	}
}

void AudioUdpDecoder::checkCompletedFramesQueue(unsigned int maxSize)
{
	while (_completedFramesQueue.size() > maxSize)
	{
		auto video_frame = (*_completedFramesQueue.begin()).second;
		delete video_frame;
		_completedFramesQueue.erase(_completedFramesQueue.begin());
	}
}