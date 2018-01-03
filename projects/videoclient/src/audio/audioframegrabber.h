#if defined(OCS_INCLUDE_AUDIO)
#ifndef AUDIOFRAMEGRABBER_H
#define AUDIOFRAMEGRABBER_H

#include <QObject>
#include <QSharedPointer>
#include <QAudioInput>
#include <QIODevice>
#include <QByteArray>
#include "videolib/src/pcmframe.h"

class AudioFrameGrabber : public QObject
{
	Q_OBJECT

public:
	AudioFrameGrabber(const QSharedPointer<QAudioInput>& input, QObject* parent) :
		QObject(parent), _input(input)
	{
		QObject::connect(_input.data(), &QAudioInput::notify, this, &AudioFrameGrabber::onInputNotify);
		_input->setNotifyInterval(11);
		_inputDevice = _input->start();
	}

	~AudioFrameGrabber()
	{
		_input->disconnect(this);
	}

protected:
	PcmFrame* createFrame(QByteArray& buffer) const
	{
		// Determine size of frame.
		const int sampleRate = 8000; //format.sampleRate();
		qint64 size = PcmFrame::calculateNumSamples(60, sampleRate) * 2;
		if (buffer.length() < size)
		{
			size = PcmFrame::calculateNumSamples(40, sampleRate) * 2;
			if (buffer.length() < size)
			{
				size = PcmFrame::calculateNumSamples(20, sampleRate) * 2;
				if (buffer.length() < size)
				{
					size = PcmFrame::calculateNumSamples(10, sampleRate) * 2;
					if (buffer.length() < size)
					{
						return nullptr;
					}
				}
			}
		}

		QByteArray desiredData = buffer.left(size);
		buffer.remove(0, size);

		// Create container for read data.
		PcmFrame* frame = new PcmFrame();
		frame->data = (char*)malloc(size);
		memcpy(frame->data, desiredData.data(), size);
		frame->numSamples = size >> 1;
		frame->numChannels = 1;
		frame->samplingRate = sampleRate;
		return frame;
	}

private:
	void onInputNotify()
	{
		_buff.append(_inputDevice->readAll());

		PcmFrame* f = nullptr;
		while ((f = createFrame(_buff)) != nullptr)
		{
			PcmFrameRefPtr p(f);
			emit newFrame(p);
		}
	}

signals:
	void newFrame(const PcmFrameRefPtr& f);

private:
	QSharedPointer<QAudioInput> _input;
	QIODevice* _inputDevice;
	QByteArray _buff;
};

#endif
#endif
