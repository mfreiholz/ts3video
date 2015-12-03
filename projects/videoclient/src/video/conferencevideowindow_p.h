#ifndef ConferenceVideoWindowPRIVATE_H
#define ConferenceVideoWindowPRIVATE_H

#include <QSharedPointer>
#include <QCamera>

#include "conferencevideowindow.h"
#include "networkclient/networkclient.h"

#if defined(OCS_INCLUDE_AUDIO)
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include "audio/audioframeplayer.h"
#endif

class ViewBase;
class ClientCameraVideoWidget;

class ConferenceVideoWindowPrivate : public QObject
{
	Q_OBJECT

public:
	ConferenceVideoWindowPrivate(ConferenceVideoWindow* o);
	~ConferenceVideoWindowPrivate();
	QSharedPointer<QCamera> createCameraFromOptions() const;

#if defined(OCS_INCLUDE_AUDIO)
	QSharedPointer<QAudioInput> createMicrophoneFromOptions() const;

	QAudioFormat createAudioFormat() const
	{
		QAudioFormat format;
		format.setSampleRate(8000);
		format.setChannelCount(1);
		format.setSampleSize(16);
		format.setCodec("audio/pcm");
		format.setByteOrder(QAudioFormat::LittleEndian);
		format.setSampleType(QAudioFormat::UnSignedInt);
		return format;
	}
#endif

public:
	ConferenceVideoWindow* owner;
	ConferenceVideoWindow::Options opts;
	QSharedPointer<NetworkClient> nc;
	QSharedPointer<QCamera> camera;

#if defined(OCS_INCLUDE_AUDIO)
	QSharedPointer<QAudioInput> audioInput;
	QSharedPointer<QAudioOutput> audioOutput;
	QSharedPointer<AudioFramePlayer> audioPlayer;
#endif

	// Direct GUI elements.
	ViewBase* view; ///< Central view to display all video streams.
	ClientCameraVideoWidget* cameraWidget; ///< Local user's camera widget.
};


#endif